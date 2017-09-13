// Do I really need a seperate file for these?

#pragma once
#include <random> 	// rand()
#include <vector> 	// randomColor()
#include <cmath>    // std::sqrt()
#include <valarray> // std::shuffle()

// Entity stuff
static int lastNodeId = 1;
struct Color { uint8_t r, g, b; };
struct Position { float x, y; };

// Converts mass to size
// Usage: toSize(mass);
static float toSize(const int &x) {
    return std::sqrt(x*100);
}

// use this instead of rand() % x
// Usage: rand(min, max);
static int rand(const int& min, const int& max) {
    std::mt19937 e{std::random_device{}()};
    std::uniform_int_distribution<int> x(min, max);
    return x(e);
}

static int getNextNodeId() {
    if (lastNodeId > 2147483647)
        lastNodeId = 1;
    return lastNodeId++;
}

static Color randomColor() {
    uint8_t RGB[3] = { 255, 7, (uint8_t)rand(0, 256) };

    std::shuffle(&RGB[0], &RGB[3], std::random_device{});
    return { RGB[0], RGB[2], RGB[1] };
}

// checkIntersection({x, y}, size, entity);
// basically same thing as canEat() in Entity.h. combine those?
static bool checkIntersection(const Position& position, const float& size, const auto &entity) {
    return std::hypot(
        position.x - entity->getPosition().x,
        position.y - entity->getPosition().y)
    <= (size + entity->getSize());
}
