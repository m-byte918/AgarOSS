#pragma once

#include <valarray>
#include "../Utils.h"

class Food;
class Virus;
class Ejected;
class MotherCell;
class PlayerCell;

namespace EntityHandler {
    extern std::vector<Food*>       _foods;
    extern std::vector<Virus*>      _viruses;
    extern std::vector<Ejected*>    _ejecteds;
    extern std::vector<MotherCell*> _mothercells;
    extern std::vector<PlayerCell*> _playercells;

    extern unsigned int prevNodeId;

    struct Color { unsigned char r, g, b; };

    extern Position getRandomPosition();
    extern Color getRandomColor();
};
