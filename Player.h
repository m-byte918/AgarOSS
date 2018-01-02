#pragma once

#include <vector>
#include <uWS/uWS.h>

#include "Utils.h"
#include "Packet.h"
#include "GameServer.h"

enum struct PlayerState { 
    DEAD,
    PLAYING,
    FREEROAM,
    SPECTATING,
    DISCONNECTED
};

class GameServer;
class Player {
    private:
        PlayerState _state = PlayerState::DEAD;
        std::string _name = "An unnamed cell";
        std::string _skin;

    public:
        GameServer *_owner;
        uWS::WebSocket<uWS::SERVER> *_socket;

        Packet   _packet = Packet();
        Position _mouse = { 0, 0 };
        unsigned int _playerId = ++prevPlayerId;

        void setState(const PlayerState &state);
        PlayerState &getState();

        void setName(const std::string &name);
        std::string &getName();

        void setSkin(const std::string &skin);
        std::string &getSkin();

        void update();
        
        Player(uWS::WebSocket<uWS::SERVER> *socket);
        ~Player();
};
