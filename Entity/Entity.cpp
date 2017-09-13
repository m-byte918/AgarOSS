#include "Entity.h"

// ====Entity====

const int Entity::getId() const {
    return _id;
}
const float Entity::getSize() const {
    return _size;
}
const float Entity::getMass() const {
    return _mass;
}
const Color Entity::getColor() const {
    return _color;
}
const Position Entity::getPosition() const {
    return _position;
}
const float Entity::getSquareSize() const {
    return _squareSize;
}

void Entity::setSize(const float &size) {
    _size        = size;
    _squareSize  = _size * _size;
    _mass        = _squareSize / 100;
}
void Entity::setPosition(const Position& position) {
    _position = position;
}
void Entity::setColor(const Color& color) {
    _color = color;
}

// *entity1 << entity2->getSize()
void Entity::operator<<(const float& size) {
    setSize(std::hypot(_size, size));
}
