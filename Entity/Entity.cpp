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

Entity::Entity(const double &size, const Color &color, const Position &position) {
    setSize(size);
    setColor(color);
    setPosition(position);
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
    setColor({ 0x33, 0xff, 0x33 });
    setPosition(getRandomPosition());
    setSize(config<double>("virusStartSize"));
}

Virus::~Virus() {

}

/********* EJECTED *********/

Ejected::Ejected(const Color &color, const Position &position) {
    setColor(color);
    setPosition(position);
    setSize(config<double>("ejectedStartSize"));
}

Ejected::~Ejected() {

}

/********* MOTHERCELL *********/

MotherCell::MotherCell() {
    setColor({ 0xce, 0x63, 0x63 });
    setPosition(getRandomPosition());
    setSize(config<double>("motherCellStartSize"));
}

MotherCell::~MotherCell() {

}

/********* PLAYERCELL *********/

PlayerCell::PlayerCell() {
    setColor(getRandomColor());
    setPosition(getRandomPosition());
    setSize(config<double>("playerCellStartSize"));
}

PlayerCell::~PlayerCell() {

}
