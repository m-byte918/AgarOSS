#pragma once

#include <valarray>
#include "../Utils.h"

class Food;
class Virus;
class Ejected;
class MotherCell;
class PlayerCell;

static std::vector<Food*>       _foods;
static std::vector<Virus*>      _viruses;
static std::vector<Ejected*>    _ejecteds;
static std::vector<MotherCell*> _mothercells;
static std::vector<PlayerCell*> _playercells;

static unsigned int prevNodeId = 0;

struct Color { unsigned char r, g, b; };

// returns random position on the map through format { x, y }
static Position getRandomPosition() {
    double halfWidth = config<double>("mapWidth") / 2;
    double halfHeight = config<double>("mapHeight") / 2;
    return {
        rand(-halfWidth, halfWidth), 
        rand(-halfHeight, halfHeight)
    };
}

static Color getRandomColor() {
    unsigned char RGB[3] = { 255, 7, (unsigned char)rand(0, 256) };

    std::shuffle(&RGB[0], &RGB[3], std::random_device{});
    return { RGB[0], RGB[2], RGB[1] };
}
