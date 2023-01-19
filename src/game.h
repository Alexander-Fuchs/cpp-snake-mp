#ifndef GAME_H
#define GAME_H

#include <random>
#include "SDL.h"
#include "controller.h"
#include "renderer.h"
#include "snake.h"

class Game {
public:
    Game(std::size_t grid_width, std::size_t grid_height);
    ~Game();

    void Run(Controller const &controller, Renderer &renderer,
             std::size_t target_frame_duration, const std::string &client_ip);

    [[nodiscard]] std::array<Snake, 2> GetSnakes() const;

private:
    Snake snake;
    Snake snake2;
    SDL_Point food;

    std::random_device dev;
    std::mt19937 engine;
    std::uniform_int_distribution<int> random_w;
    std::uniform_int_distribution<int> random_h;

    void PlaceFood(int forceX = -1, int forceY = -1);
    void SetupSocket(const std::string& client_ip = "");
    void Update();
    static void *SocketHandler(void* game_ptr);

    bool isHost{false};
    int server_fd, client_fd, _socket;
};

#endif