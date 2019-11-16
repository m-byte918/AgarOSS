#pragma once
#pragma warning(push, 0)      
#include <uwebsockets/App.h>
#pragma warning(pop)
#include <thread>

class Player;
class Minion;
class PlayerBot;
struct Server {
    std::vector<Player*> clients;
    std::vector<Minion*> minions;
    std::vector<PlayerBot*> playerBots;

    unsigned long long connections = 0;
    int runningState = -1;

    void start();
    void end();

private:
    // Thread that uWebSockets will run on
    std::thread connectionThread;
};