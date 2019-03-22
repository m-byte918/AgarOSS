#include "PlayerBot.hpp"
#include "../Game/Map.hpp"
#include "../Game/Game.hpp"
#include "../Modules/Logger.hpp"

PlayerBot::PlayerBot(Server *_server) :
    Player(_server) {
    server->playerBots.push_back(this);
}

void PlayerBot::update(unsigned long long tick) {
    if (_state == PlayerState::DEAD)
        spawn(cellName());
    else if (_state == PlayerState::PLAYING)
        _mouse = owner->mouse();
    else if (_state == PlayerState::DISCONNECTED)
        updateDisconnection(tick);
}
void PlayerBot::updateDisconnection(unsigned long long tick) {
    if (disconnectionTick == 0)
        disconnectionTick = tick;
    if (cfg::player_cellRemoveTime <= 0 || cells.empty()) {
        server->playerBots.erase(std::find(server->playerBots.begin(), server->playerBots.end(), this));
    } else if (tick - disconnectionTick >= cfg::player_cellRemoveTime * 25) {
        while (!cells.empty())
            map::despawn(cells.back());
    }
}

PlayerBot::~PlayerBot() {
}
