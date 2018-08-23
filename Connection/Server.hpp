#pragma once
#pragma warning(push, 0)        
#include <uWS/Hub.h>
#pragma warning(pop)
#include <thread>

// might not need a struct for this
struct Server {
    std::vector<uWS::WebSocket<uWS::SERVER>*> clients;
    unsigned long long connections = 0;

    void start();
    void onClientConnection(uWS::Hub *hub);
    void onClientDisconnection(uWS::Hub *hub);
    void onClientMessage(uWS::Hub *hub);
    void end();

private:
    std::thread connectionThread;
};