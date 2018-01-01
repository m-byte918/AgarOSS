#include "Player.h"

Player::Player(uWS::WebSocket<uWS::SERVER> *socket) { 
    _socket = socket;
    _packetHandler._socket = _socket;
}

Player::~Player() { 

}

void Player::update() {
    if (_mouse != _packetHandler._target)
        _mouse = _packetHandler._target;
}
