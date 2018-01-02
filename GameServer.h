#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <uWS/uWS.h>

#include "Utils.h"
#include "Modules/Logger.h"

class GameServer {
    private:
        std::vector<uWS::WebSocket<uWS::SERVER>*> _clients;

        uWS::Hub _hub;
        Timer *_timerLoop;
        unsigned long long _tickCount = 0;

        void run();
        void updateFood();
        void onClientConnection();
        void onClientDisconnection();
        void onClientMessage();
        void onServerSocketError();
        void startServer();

        unsigned int _serverConnections = 0;

    public:
        GameServer();
        ~GameServer();

        struct { double minx, miny, maxx, maxy; } _border;
};
