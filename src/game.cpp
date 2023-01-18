#include "game.h"
#include "SDL.h"
#include <iostream>
#include <array>
#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
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
    PlaceFood(30, 30);
}

void Game::SetupSocket(const std::string& server_ip) {
    int server_fd, client_fd, new_socket;
    int opt = 1;
    struct sockaddr_in address;
    if (!server_ip.empty()) {
        printf("Server IP not empty... using it\n");
        inet_pton(AF_INET, server_ip.c_str(), &address.sin_addr);
    }
    int addrlen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server_fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    if (isHost) {
        address.sin_addr.s_addr = INADDR_ANY;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }
        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }
        if (listen(server_fd, 3) < 0) {
            perror("listen");
            exit(EXIT_FAILURE);
        }
        printf("before accept\n");
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) > 0) {
            _socket = new_socket;
            pthread_t socket_handler;
            pthread_create(&socket_handler, NULL, Game::SocketHandler, this);
        }
    } else {
        if (inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) <= 0) {
            printf("Invalid address / address not supported!\n");
            return;
        }
        if ((client_fd = connect(server_fd, (struct sockaddr*)&address, addrlen)) < 0) {
            printf("Connection failed!\n");
            return;
        }
        _socket = server_fd;
        pthread_t socket_handler;
        pthread_create(&socket_handler, NULL, Game::SocketHandler, this);
    }
}

void *Game::SocketHandler(void *game_ptr)
{
    printf("SocketHandler\n");
    Game * game = (Game*)game_ptr;
    int socket = game->_socket;
    char buffer[64] = { 0 };
    while (read(socket, buffer, 64)) {
        std::string strBuffer = buffer;
        // printf("%s\n", buffer);
        if (strBuffer.substr(0, 1) == "s") {
            if (game->isHost) {
                bool eaten = game->snake2.HasEaten(game->food, true);
                game->PlaceFood();
                std::string message = "f" + std::to_string(game->food.x) + "," + std::to_string(game->food.y);
                send(socket, static_cast<void*>(&message), message.size(), 0);
            } else {
                bool eaten = game->snake.HasEaten(game->food, true);
            }
            strBuffer.erase(0, 1);
        }
        if (strBuffer.substr(0, 1) == "d") {
            int direction = stoi(strBuffer.substr(1, 1));
            auto new_direction = static_cast<Snake::Direction>(direction);
            if (game->isHost) {
                game->snake2.direction = new_direction;
            } else {
                game->snake.direction = new_direction;
            }
            strBuffer.erase(0, 2);
        }
        if (strBuffer.substr(0, 1) == "f") {
            if (!game->isHost) {
                strBuffer.erase(0, 1);
                std::string token;
                unsigned long delPos = strBuffer.find(',');
                token = strBuffer.substr(0, delPos);
                int x = strtol(token.c_str(), NULL, 10);
                strBuffer.erase(0, delPos + 1);
                int y = strtol(strBuffer.c_str(), NULL, 10);
                game->food.x = x;
                game->food.y = y;
            }
        }
        if (strBuffer.substr(0, 1) == "q") {
            // TODO: Quit game
        }
        memset(buffer, '\0', sizeof(buffer));
    }
}

void Game::Run(Controller const &controller, Renderer &renderer,
               std::size_t target_frame_duration, const std::string &server_ip) {

    if (server_ip.empty()) {
        isHost = true;
        printf("Starting server...\n");
        SetupSocket();
    } else {
        isHost = false;
        printf("Connecting to server... %s\n", server_ip.c_str());
        SetupSocket(server_ip);
    }

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
            controller.HandleInput(running, snake, _socket);
        } else {
            controller.HandleInput(running, snake2, _socket);
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

void Game::PlaceFood(int forceX, int forceY) {
    int x, y;
    if (forceX >= 0 && forceY >= 0) {
        food.x = forceX;
        food.y = forceY;
    } else {
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
}

void Game::Update() {
    snake.Update();
    snake2.Update();
    if (isHost) {
        bool snake1Eaten = snake.HasEaten(food);
        if (snake1Eaten) {
            send(_socket, "s", 1, 0);
            PlaceFood();
            std::string message = "f" + std::to_string(food.x) + "," + std::to_string(food.y);
            send(_socket, static_cast<void*>(&message), message.size(), 0);
        }
    } else {
        bool snake2Eaten = snake2.HasEaten(food);
        if (snake2Eaten) {
            send(_socket, "s", 1, 0);
        }
    }
}