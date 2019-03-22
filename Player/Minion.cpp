#include "Minion.hpp"
#include "../Game/Map.hpp"
#include "../Game/Game.hpp"
#include "../Modules/Logger.hpp"

Minion::Minion(Server *_server, Player *_owner) : 
    Player(_server) {
    owner = _owner;
    server->minions.push_back(this);
}

void Minion::update(unsigned long long tick) {
    if (_state == PlayerState::DEAD)
        spawn(cellName());
    else if (_state == PlayerState::PLAYING)
        _mouse = owner->mouse();
    else if (_state == PlayerState::DISCONNECTED)
        updateDisconnection(tick);
}
void Minion::updateDisconnection(unsigned long long tick) {
    if (disconnectionTick == 0)
        disconnectionTick = tick;
    if (cfg::player_cellRemoveTime <= 0 || cells.empty()) {
        server->minions.erase(std::find(server->minions.begin(), server->minions.end(), this));
    } else if (tick - disconnectionTick >= cfg::player_cellRemoveTime * 25) {
        while (!cells.empty())
            map::despawn(cells.back());
    }
}
void Minion::onDisconnection() noexcept {
    Player::onDisconnection();
    owner->minions.erase(std::find(owner->minions.begin(), owner->minions.end(), this));
}
Minion::~Minion() {
}
