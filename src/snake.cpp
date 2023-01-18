#include "snake.h"
#include <cmath>
#include <iostream>

Snake::Snake(int grid_width, int grid_height, bool is_client_player)
        : grid_width(grid_width),
          grid_height(grid_height),
          is_client_player(is_client_player) {
    head_x = static_cast<int>(grid_width / 2);
    head_y = static_cast<int>(grid_height / 2);
    if (is_client_player) {
        direction = Direction::kUp;
    }
}

void Snake::Update() {
    SDL_Point prev_cell{
            static_cast<int>(head_x),
            static_cast<int>(head_y)};  // We first capture the head's cell before updating.
    UpdateHead();
    SDL_Point current_cell{
            static_cast<int>(head_x),
            static_cast<int>(head_y)};  // Capture the head's cell after updating.

    // Update all of the body vector items if the snake head has moved to a new cell.
    if (current_cell.x != prev_cell.x || current_cell.y != prev_cell.y) {
        UpdateBody(current_cell, prev_cell);
    }
}

void Snake::UpdateHead() {
    switch (direction) {
        case Direction::kUp:
            head_y -= speed;
            break;

        case Direction::kDown:
            head_y += speed;
            break;

        case Direction::kLeft:
            head_x -= speed;
            break;

        case Direction::kRight:
            head_x += speed;
            break;
    }

    // Wrap the Snake around to the beginning if going off of the screen.
    head_x = fmod(head_x + grid_width, grid_width);
    head_y = fmod(head_y + grid_height, grid_height);
}

void Snake::UpdateBody(SDL_Point &current_head_cell, SDL_Point &prev_head_cell) {
    // Add previous head location to vector
    body.push_back(prev_head_cell);

    if (!growing) {
        // Remove the tail from the vector.
        body.erase(body.begin());
    } else {
        growing = false;
        size++;
    }

    // Check if the snake has died.
    for (auto const &item: body) {
        if (current_head_cell.x == item.x && current_head_cell.y == item.y) {
            alive = false;
            // TODO: Tell client that this snake has died.
        }
    }
}

bool Snake::HasEaten(SDL_Point &food, bool force) {
    int new_x = static_cast<int>(head_x);
    int new_y = static_cast<int>(head_y);
    // Check if there's food over here
    if ((food.x == new_x && food.y == new_y) || force) {
        score++;
        // Grow snake and increase speed.
        GrowBody();
        speed += 0.02;
        return true;
    }
    return false;
}

void Snake::GrowBody() { growing = true; }

void Snake::SetScore(int newScore) {
    score = newScore;
}

int Snake::GetScore() const { return score; }

int Snake::GetSize() const { return size; }

// Inefficient method to check if cell is occupied by snake.
bool Snake::SnakeCell(int x, int y) {
    if (x == static_cast<int>(head_x) && y == static_cast<int>(head_y)) {
        return true;
    }
    for (auto const &item: body) {
        if (x == item.x && y == item.y) {
            return true;
        }
    }
    return false;
}
