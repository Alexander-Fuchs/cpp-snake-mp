#include "game.h"
#include "SDL.h"
#include <iostream>
#include <array>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <functional>
#include <unistd.h>
#define PORT 65010

Game::Game(std::size_t grid_width, std::size_t grid_height)
        : snake(grid_width, grid_height),
          snake2(grid_width, grid_height, true),
          engine(dev()),
          random_w(0, static_cast<int>(grid_width - 1)),
          random_h(0, static_cast<int>(grid_height - 1)) {
    PlaceFood();
}

void Game::SetupSocket() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) > 0) {
        _socket = new_socket;
        pthread_t socket_handler;
        pthread_create(&socket_handler, NULL, Game::SocketHandler, this);
    }
}

void *Game::SocketHandler(void *game_ptr)
{
    printf("SocketHandler\n");
    Game * game = (Game*)game_ptr;
    int socket = game->_socket;
    char buffer[1024] = { 0 };
    const char* hello = "Hello from server";
    while (read(socket, buffer, 1024)) {
        printf("%s\n", buffer);
        send(socket, hello, strlen(hello), 0);
        printf("Hello message sent\n");
    }
}

void Game::Run(Controller const &controller, Renderer &renderer,
               std::size_t target_frame_duration, const std::string &client_ip) {

    if (client_ip.empty()) {
        isHost = true;
        printf("Starting server...\n");
        // TODO: Wait for connection...
        SetupSocket();
    } else {
        printf("Connecting to server... %s\n", client_ip.c_str());
        // TODO: Wait for connection...
    }

    // TODO: As host: When getting message to add up snake2 score place new food and send position to client

    Uint32 title_timestamp = SDL_GetTicks();
    Uint32 frame_start;
    Uint32 frame_end;
    Uint32 frame_duration;
    int frame_count = 0;
    bool running = true;

    while (running) {
        if (!snake.alive || !snake2.alive) break;

        frame_start = SDL_GetTicks();

        // Input, Update, Render - the main game loop.
        if (isHost) {
            controller.HandleInput(running, snake);
        } else {
            controller.HandleInput(running, snake2);
        }
        Update();
        renderer.Render(snake, snake2, food);

        frame_end = SDL_GetTicks();

        // Keep track of how long each loop through the input/update/render cycle takes.
        frame_count++;
        frame_duration = frame_end - frame_start;

        // After every second, update the window title.
        if (frame_end - title_timestamp >= 1000) {
            renderer.UpdateWindowTitle(snake, snake2, frame_count);
            frame_count = 0;
            title_timestamp = frame_end;
        }

        // If the time for this frame is too small (i.e. frame_duration is
        // smaller than the target ms_per_frame), delay the loop to achieve the correct frame rate.
        if (frame_duration < target_frame_duration) {
            SDL_Delay(target_frame_duration - frame_duration);
        }
    }
}

std::array<Snake, 2> Game::GetSnakes() const {
    std::array<Snake, 2> snake_array = {snake, snake2};
    return snake_array;
}

void Game::PlaceFood() {
    int x, y;
    while (true) {
        x = random_w(engine);
        y = random_h(engine);
        // Check that the location is not occupied by a snake item before placing food.
        if (!snake.SnakeCell(x, y) && !snake2.SnakeCell(x, y)) {
            food.x = x;
            food.y = y;
            return;
        }
    }
}

void Game::Update() {
    snake.Update();
    snake2.Update();

    if (isHost) {
        bool snake1Eaten = snake.HasEaten(food);
        if (snake1Eaten) {
            send(_socket, "eaten", strlen("eaten"), 0);
            // TODO: Send to client to add up snake score
            PlaceFood();
            // TODO: Send to client new food position
        }
    } else {
        bool snake2Eaten = snake2.HasEaten(food);
        if (snake2Eaten) {
            // TODO: Send to host to add up snake2 score
        }
    }
}