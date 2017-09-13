#pragma once
#include "Entity/Entity.h"

/* Master configuration file */

namespace cfg {

    // Amount of server-updates per second
    int tickStep = 40;

    // Port used to listen for incoming connections
    int serverPort = 3000;
	
    // Maximum connections allowed to server
    int maxConnections = 500;

    // Map width and height
    float mapWidth = 14142.135623730952;
    float mapHeight = 14142.135623730952;

    int foodStartAmount = 2000;
    int virusStartAmount = 100;

    // Default starting size for entities
    // toSize(mass)
    float foodStartSize   = 10.0;
    float virusStartSize  = 100.0;
    float motherStartSize = 141.42;
    float playerStartSize = 32.0;

    // Default virus & mothercell colors
    // Format: { r, g, b }
    Color virusColor = { 51, 255, 51 };
    Color motherColor = { 206, 99, 99 };

}

/*
Old method (loading from file)
Syntax: config<type>("configname");

template <class T>
T config(const std::string &name) {
    std::string line;
    std::ifstream in("config.ini");
    while (std::getline(in, line)) {
        std::istringstream sin(line.substr(line.find("=") + 1));

        if (line.find(name) != -1) {
            T value;
            sin >> value;
            return value;
        }
    }
}
*/
