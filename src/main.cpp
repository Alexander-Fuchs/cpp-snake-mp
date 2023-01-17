#include <iostream>
#include <array>
#include "controller.h"
#include "game.h"
#include "renderer.h"

int main(int argc, char **argv) {
    constexpr std::size_t kFramesPerSecond{60};
    constexpr std::size_t kMsPerFrame{1000 / kFramesPerSecond};
    constexpr std::size_t kScreenWidth{640};
    constexpr std::size_t kScreenHeight{640};
    constexpr std::size_t kGridWidth{32};
    constexpr std::size_t kGridHeight{32};

    std::string client_ip;
    if (argc == 1) {
        std::cout << "You've decided to host a game, please wait for incoming connections...\n";
        std::cout << "You can also connect to a game by running the program with the IP address of the host as an argument.\n";
    } else if (argc == 2) {
        client_ip = argv[1];
        std::cout << "Trying to connect to host...\n";
    }

    Renderer renderer(kScreenWidth, kScreenHeight, kGridWidth, kGridHeight);
    Controller controller;
    Game game(kGridWidth, kGridHeight);
    game.Run(controller, renderer, kMsPerFrame, client_ip);

    std::array<Snake, 2> snakes = game.GetSnakes();

    std::cout << "Game has terminated successfully!\n";
    std::cout << "Score Host: " << snakes[0].GetScore() << "\n";
    std::cout << "Score Client: " << snakes[1].GetScore() << "\n";
    return 0;
}