#pragma once
#include "Player.hpp"

class PlayerBot : public Player {
public:
    PlayerBot(Server *_server);

    void update(unsigned long long tick);
    void updateDisconnection(unsigned long long tick);

    ~PlayerBot();
};