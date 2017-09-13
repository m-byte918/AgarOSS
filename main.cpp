#include <iostream>
#include "GameServer.h"

// do cool stuffs with c++17 later
// g++ -std=c++17 main.cpp Entity/Entity.cpp -luWS -luv -lz -lssl

// food information
void printFoodInfo() {
    for (int i = 0; i < _food.size(); ++i) {
        std::cout << "**********_food[" << i << "]********** \n";
        std::cout << "getId(): " << _food[i]->getId() << "\n";
        std::cout << "getSize(): " << _food[i]->getSize() << "\n";
        std::cout << "getMass(): " << _food[i]->getMass() << "\n";
        std::cout << "getSquareSize(): " << _food[i]->getSquareSize() << "\n";
        std::cout << "getColor(): { ";
        std::cout << (int)_food[i]->getColor().r << ", ";
        std::cout << (int)_food[i]->getColor().g << ", ";
        std::cout << (int)_food[i]->getColor().b << " ";
        std::cout << "} \n";
        std::cout << "getPosition(): { ";
        std::cout << _food[i]->getPosition().x << ", ";
        std::cout << _food[i]->getPosition().y << " ";
        std::cout << "} \n";
        std::cout << "--------------------------------\n";
    }
}

// virus information
void printVirusInfo() {
    for (int i = 0; i < _viruses.size(); ++i) {
        std::cout << "********_viruses[" << i << "]********* \n";
        std::cout << "getId(): " << _viruses[i]->getId() << "\n";
        std::cout << "getSize(): " << _viruses[i]->getSize() << "\n";
        std::cout << "getMass(): " << _viruses[i]->getMass() << "\n";
        std::cout << "getSquareSize(): " << _viruses[i]->getSquareSize() << "\n";
        std::cout << "getColor(): { ";
        std::cout << (int)_viruses[i]->getColor().r << ", ";
        std::cout << (int)_viruses[i]->getColor().g << ", ";
        std::cout << (int)_viruses[i]->getColor().b << " ";
        std::cout << "} \n";
        std::cout << "getPosition(): { ";
        std::cout << _viruses[i]->getPosition().x << ", ";
        std::cout << _viruses[i]->getPosition().y << " ";
        std::cout << "} \n";
        std::cout << "--------------------------------\n";
    }
}

int main() {

    Game *server = new Game();

    delete server;
}
