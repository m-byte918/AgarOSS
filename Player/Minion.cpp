#include "Minion.hpp"
#include "../Game/Map.hpp"
#include "../Game/Game.hpp"
#include "../Modules/Logger.hpp"

Minion::Minion(Server *_server, Player *_owner) : 
    Player(_server) {
    owner = _owner;
    server->minions.push_back(this);
}

void Minion::update() {
    if (_state == PlayerState::DEAD) {
        // Only spawn if owner is in-game
        if (owner->state() == PlayerState::PLAYING)
            spawn();
    } else if (_state == PlayerState::PLAYING) {
        _mouse = owner->mouse();
    }
}
void Minion::onDisconnection() noexcept {
    Player::onDisconnection();
    owner->minions.erase(std::find(owner->minions.begin(), owner->minions.end(), this));
}

Minion::~Minion() {
}
