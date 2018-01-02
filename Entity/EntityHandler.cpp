#include "EntityHandler.h"

namespace EntityHandler {
    std::vector<Food*>       _foods;
    std::vector<Virus*>      _viruses;
    std::vector<Ejected*>    _ejecteds;
    std::vector<MotherCell*> _mothercells;
    std::vector<PlayerCell*> _playercells;

    unsigned int prevNodeId = 0;

    // returns random position on the map through format { x, y }
    Position getRandomPosition() {
        double halfWidth = config<double>("mapWidth") / 2;
        double halfHeight = config<double>("mapHeight") / 2;
        return {
            rand(-halfWidth, halfWidth), 
            rand(-halfHeight, halfHeight)
        };
    }

    Color getRandomColor() {
        unsigned char RGB[3] = { 255, 7, (unsigned char)rand(0, 256) };

        std::shuffle(&RGB[0], &RGB[3], std::random_device{});
        return { RGB[0], RGB[2], RGB[1] };
    }
}
