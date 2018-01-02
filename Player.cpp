#include "Player.h"

Player::Player(uWS::WebSocket<uWS::SERVER> *socket) { 
    _socket = socket;
    _packet._player = this;
}

Player::~Player() { 

}

void Player::setState(const PlayerState &state) {
    _state = state;
}
PlayerState &Player::getState() {
    return _state;
}

void Player::setName(const std::string &name) {
    if (_name == name) 
        return;
    if (name.size() == 0 || (name.size() == 1 && name[0] == 0))
        _name = "An unnamed cell";
    else
        _name = name;
}
std::string &Player::getName() {
    return _name;
}

void Player::setSkin(const std::string &skin) {
    if (_skin == skin)
        return;
    _skin = skin.size() == 0 ? "" : skin;
}
std::string &Player::getSkin() {
    return _skin;
}

void Player::update() {
}
