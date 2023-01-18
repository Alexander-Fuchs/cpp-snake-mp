#include "controller.h"
#include <sys/socket.h>
#include <iostream>
#include "SDL.h"
#include "snake.h"

void Controller::ChangeDirection(Snake &snake, Snake::Direction input,
                                 Snake::Direction opposite, int &socket) const {
    if (snake.direction == input) {
        return;
    }
    if (snake.direction != opposite || snake.size == 1) {
        snake.direction = input;
        std::string message = "d" + std::to_string((int)snake.direction);
        send(socket, static_cast<void*>(&message), message.size(), 0);
    }
}

void Controller::HandleInput(bool &running, Snake &snake, int &socket) const {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            running = false;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_UP:
                    ChangeDirection(snake, Snake::Direction::kUp,Snake::Direction::kDown, socket);
                    break;
                case SDLK_DOWN:
                    ChangeDirection(snake, Snake::Direction::kDown,Snake::Direction::kUp, socket);
                    break;
                case SDLK_LEFT:
                    ChangeDirection(snake, Snake::Direction::kLeft,Snake::Direction::kRight, socket);
                    break;
                case SDLK_RIGHT:
                    ChangeDirection(snake, Snake::Direction::kRight,Snake::Direction::kLeft, socket);
                    break;
            }
        }
    }
}