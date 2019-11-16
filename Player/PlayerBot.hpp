#pragma once
#include "Player.hpp"

class PlayerBot : public Player {
public:
    PlayerBot(Server *_server);

    void update();
    void updateVisibleNodes();
    void decide(sptr<PlayerCell::Entity> largestCell);

    ~PlayerBot();
private:
    int splitCooldown = 0;
    std::vector<e_ptr> visibleNodes;
};