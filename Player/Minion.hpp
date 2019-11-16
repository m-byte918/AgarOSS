#pragma once
#include "Player.hpp"

class Minion : public Player {
public:
    Minion(Server *_server, Player *_owner);

    void update();
    void onDisconnection() noexcept;

    ~Minion();
};