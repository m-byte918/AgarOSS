#pragma once

#include "Server.hpp"
#include "Modules/Commands.hpp"

class Game {
    friend struct Commands;
public:
    Game();
    void mainLoop();
    ~Game();

private:
    Server server;
    bool running = true;
    unsigned long long tickCount = 0;
};