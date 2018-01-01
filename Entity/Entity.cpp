#include "Entity.h"

/********* ENTITY *********/

const unsigned int &Entity::getNodeId() const {
    return _nodeId;
}

void Entity::setPosition(const Position &position) {
    if (_position.x == position.x && _position.y == position.y)
        return;
    _position = position;
}
const Position &Entity::getPosition() const {
    return _position;
}

void Entity::setColor(const Color &color) {
    _color = color;
}
const Color &Entity::getColor() const {
    return _color;
}

void Entity::setSize(const double &size) {
    _size = size;
    _sizeSquared = size * size;
    _mass = _sizeSquared / 100;
}
const double &Entity::getSize() const {
    return _size;
}
const double &Entity::getSizeSquared() const {
    return _sizeSquared;
}
const double &Entity::getMass() const {
    return _mass;
}

Entity::Entity() {

}

Entity::~Entity() {

}

/********* FOOD *********/

Food::Food() {
    setColor(getRandomColor());
    setPosition(getRandomPosition());
    setSize(config<double>("foodStartSize"));
}

Food::~Food() {
	
}

/********* VIRUS *********/

Virus::Virus() {

}

Virus::~Virus() {

}

/********* EJECTED *********/

Ejected::Ejected() {
	
}

Ejected::~Ejected() {

}

/********* MOTHERCELL *********/

MotherCell::MotherCell() {

}

MotherCell::~MotherCell() {

}

/********* PLAYERCELL *********/

PlayerCell::PlayerCell() {

}

PlayerCell::~PlayerCell() {

}
