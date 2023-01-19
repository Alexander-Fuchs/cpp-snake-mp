#ifndef SNAKE_H
#define SNAKE_H

#include <vector>
#include "SDL.h"

class Snake {
public:
    enum class Direction {
        kUp, kDown, kLeft, kRight
    };

    Snake(int grid_width, int grid_height, bool is_client_player = false);

    void Update(int &socket);
    void GrowBody();
    void SetScore(int newScore);

    bool SnakeCell(int x, int y);
    bool HasEaten(SDL_Point &food, bool force = false);

    [[nodiscard]] int GetScore() const;
    [[nodiscard]] int GetSize() const;

    Direction direction = Direction::kDown;

    float speed{0.1f};
    int size{1};
    bool alive{true};
    bool is_client_player{false};
    float head_x;
    float head_y;
    std::vector<SDL_Point> body;

private:
    void UpdateHead();

    bool UpdateBody(SDL_Point &current_cell, SDL_Point &prev_cell);

    bool growing{false};
    int grid_width;
    int grid_height;
    int score{0};
};

#endif