#pragma once

#include <vector>
#include <memory>
#include <uWS/uWS.h>

#include "Utils.h"
#include "PacketHandler.h"

class Player {
    private:
        unsigned int _playerId;
        uWS::WebSocket<uWS::SERVER> *_socket;

    public:
        Position _mouse = { 0, 0 };

        PacketHandler _packetHandler = PacketHandler();

        Player(uWS::WebSocket<uWS::SERVER> *socket);
        ~Player();

        void update();
};
