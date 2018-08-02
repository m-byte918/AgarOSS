#include "Entities.hpp"
#include "../Player.hpp"
#include "Map.hpp"

/********* ENTITY *********/

const unsigned int &Entity::getNodeId() const noexcept {
    return nodeId;
}
const unsigned int &Entity::getKillerId() const noexcept {
    return killerId;
}

void Entity::setPosition(const Vector2 &_position) noexcept {
    if (position == _position) return;
    position = _position;
}
const Vector2 &Entity::getPosition() const noexcept {
    return position;
}

void Entity::setColor(const Color &_color) noexcept {
    color = _color;
}
const Color &Entity::getColor() const noexcept {
    return color;
}

void Entity::setSize(double _size) noexcept {
    size = _size;
    sizeSquared = _size * _size;
    mass = sizeSquared / 100;
}
double Entity::getSize() const noexcept {
    return size;
}
double Entity::getSizeSquared() const noexcept {
    return sizeSquared;
}
double Entity::getMass() const noexcept {
    return mass;
}

void Entity::move() {
}
void Entity::onRemove() {
}
void Entity::updateDecay() {
}

void Entity::consume(Entity *other) {
    other->killerId = nodeId;
    setSize(std::sqrt(sizeSquared + other->sizeSquared));
}
bool Entity::intersects(Entity *other) const noexcept {
    return intersects(other->getPosition(), other->getSize());
}

bool Entity::intersects(const Vector2 &pos, double _size) const noexcept {
    /*double dx = getPosition().x - pos.x;
    double dy = getPosition().y - pos.y;
    return rs >= std::sqrt(dx * dx + dy * dy);*/
    double rs = size + _size;
    return (position - pos).squared() < (rs * rs);
}

Entity::Entity(const Vector2 &pos, double _size, const Color &_color) noexcept {
    setPosition(pos);
    setSize(_size);
    setColor(_color);
    isRemoved = false;
    killerId = 0;
}
Entity::~Entity() {
}

/********* FOOD *********/

Food::Food(const Vector2 &pos, double _size, const Color &_color) noexcept :
    Entity(pos, _size, _color) {
    cellType = CellType::FOOD;
    isSpiked = config["food"]["isSpiked"];
    isAgitated = config["food"]["isAgitated"];
}
void Food::onRemove() {
    Map::spawnFood(); // Spawn a new one
}
Food::~Food() {
}

/********* VIRUS *********/

Virus::Virus(const Vector2 &pos, double _size, const Color &_color) noexcept :
    Entity(pos, _size, _color) {
    cellType = CellType::VIRUS;
    isSpiked = config["virus"]["isSpiked"];
    isAgitated = config["virus"]["isAgitated"];
}
void Virus::onRemove() {
    Map::spawnVirus(); // Spawn a new one
}
Virus::~Virus() {
}

/********* EJECTED *********/

Ejected::Ejected(const Vector2 &pos, double _size, const Color &_color) noexcept :
    Entity(pos, _size, _color) {
    cellType = CellType::EJECTED;
    isSpiked = config["ejected"]["isSpiked"];
    isAgitated = config["ejected"]["isAgitated"];
}
Ejected::~Ejected() {
}

/********* MOTHERCELL *********/

MotherCell::MotherCell(const Vector2 &pos, double _size, const Color &_color) noexcept :
    Entity(pos, _size, _color) {
    cellType = CellType::MOTHERCELL;
    isSpiked = config["motherCell"]["isSpiked"];
    isAgitated = config["motherCell"]["isAgitated"];
}
void MotherCell::onRemove() {
    Map::spawnMotherCell(); // Spawn a new one
}
MotherCell::~MotherCell() {
}

/********* PLAYERCELL *********/

PlayerCell::PlayerCell(const Vector2 &pos, double _size, const Color &_color) noexcept :
    Entity(pos, _size, _color) {
    cellType = CellType::PLAYERCELL;
    isSpiked = config["playerCell"]["isSpiked"];
    isAgitated = config["playerCell"]["isAgitated"];
}
void PlayerCell::setSize(double _size) noexcept {
    Entity::setSize(_size);
    owner->updateScale();
}
void PlayerCell::move() {
    Vector2 d = owner->getMouse() - position;

    double squared = d.squared();
    if (squared < 1) return;

    double distance = std::sqrt(squared);
    double invd = 1 / distance;

    // normalized distance (0..1)
    distance = std::min(distance, 32.0) / 32;

    double speed = (2.2 * std::pow(size, -0.439)) * 40;
           speed *= distance;

    if (speed > 0) 
        position += (d * invd) * speed;
}
void PlayerCell::updateDecay() {
    double rate = config["playerCell"]["decayRate"];

    if (rate <= 0 || size <= (double)config["playerCell"]["minSize"])
        return;

    setSize(std::sqrt(sizeSquared * (1 - rate)));
}

PlayerCell::~PlayerCell() {
}
