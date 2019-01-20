#pragma once
#pragma warning(push, 0)        
#include <uWS/Hub.h>
#pragma warning(pop)
#include <thread>

class Player;
struct Server {
    std::vector<Player*> clients;
    unsigned long long connections = 0;
    int runningState = -1;

    void start();
    void onClientConnection(uWS::Hub *hub);
    void onClientDisconnection(uWS::Hub *hub);
    void onClientMessage(uWS::Hub *hub);
    void end();

private:
    std::thread connectionThread;
};