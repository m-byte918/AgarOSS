#include "Entities.hpp"
#include "../Player.hpp"
#include "Map.hpp"

/********* ENTITY *********/

const unsigned int &Entity::nodeId() const noexcept {
    return _nodeId;
}
const unsigned int &Entity::killerId() const noexcept {
    return _killerId;
}

void Entity::setPosition(const Vector2 &position) noexcept {
    _position = position;
}
const Vector2 &Entity::getPosition() const noexcept {
    return _position;
}

void Entity::setColor(const Color &color) noexcept {
    _color = color;
}
const Color &Entity::getColor() const noexcept {
    return _color;
}

void Entity::setMass(double mass) noexcept {
    _mass = mass;
    _radius = toRadius(mass);
}
void Entity::setRadius(double radius) noexcept {
    _radius = radius;
    _mass = toMass(radius);
}
double Entity::getRadius() const noexcept {
    return _radius;
}
double Entity::radiusSquared() const noexcept {
    return _radius * _radius;
}
double Entity::getMass() const noexcept {
    return _mass;
}

void Entity::move() {
}
void Entity::update(unsigned long long tick) {
}
void Entity::onRemove() {
}

void Entity::consume(Entity *prey) {
    if (!(canEat & prey->flag)) return; // Not allowed to eat prey

    // https://gist.github.com/Megabyte918/0b921e69f9d84b3ea7b8fdebef4f6812#file-gameconfiguration-json-L178
    // assuming "percentageOfCellToSquash" is the range required to eat another cell
    double range = _radius - 0.4 * prey->_radius;
    if ((_position - prey->_position).squared() >= range * range)
        return; // Not close enough to eat

    prey->_killerId = _nodeId; // prey was killed by this
    setMass(_mass + prey->_mass); // add prey's mass to this
    map::despawn(prey); // remove prey from map
}
bool Entity::intersects(Entity *other) const noexcept {
    return intersects(other->_position, other->_radius);
}

bool Entity::intersects(const Vector2 &pos, double radius) const noexcept {
    /*double dx = getPosition().x - pos.x;
    double dy = getPosition().y - pos.y;
    return rs >= std::sqrt(dx * dx + dy * dy);*/
    double rs = _radius + radius;
    return (_position - pos).squared() < (rs * rs);
}

Entity::Entity(const Vector2 &pos, double radius, const Color &color) noexcept {
    setPosition(pos);
    setRadius(radius);
    setColor(color);
}

Entity::~Entity() {
}

/********* FOOD *********/

Food::Food(const Vector2 &pos, double radius, const Color &color) noexcept :
    Entity(pos, radius, color) {
    type = CellType::FOOD;

    flag = food;
    canEat = config["food"]["canEat"];
    avoidSpawningOn = config["food"]["avoidSpawningOn"];
    
    isSpiked = config["food"]["isSpiked"];
    isAgitated = config["food"]["isAgitated"];
}

void Food::update(unsigned long long tick) {
    if (!canGrow || _radius >= maxRadius) return;

    // 1 out of 10 chance to grow every minute
    if ((tick % 1500) == 0 && rand(0, 10) == 10)
        setMass(_mass + 1); // setMass might be faster in this case
}
void Food::onRemove() {
    // Vanilla servers spawn new food as soon as one is eaten, so lets do that
    if (map::entities[CellType::FOOD].size() < (unsigned)config["food"]["maxAmount"])
        map::spawn<Food>(randomPosition(), config["food"]["baseRadius"], randomColor());
}
Food::~Food() {
}

/********* VIRUS *********/

Virus::Virus(const Vector2 &pos, double radius, const Color &color) noexcept :
    Entity(pos, radius, color) {
    type = CellType::VIRUS;

    flag = viruses;
    canEat = config["virus"]["canEat"];
    avoidSpawningOn = config["virus"]["avoidSpawningOn"];

    isSpiked = config["virus"]["isSpiked"];
    isAgitated = config["virus"]["isAgitated"];
}
void Virus::onRemove() {
    // Same thing applies to viruses
    if (map::entities[CellType::VIRUS].size() < (unsigned)config["virus"]["maxAmount"])
        map::spawn<Virus>(randomPosition(), config["virus"]["baseRadius"], config["virus"]["color"]);
}
Virus::~Virus() {
}

/********* EJECTED *********/

Ejected::Ejected(const Vector2 &pos, double radius, const Color &color) noexcept :
    Entity(pos, radius, color) {
    type = CellType::EJECTED;

    flag = ejected;
    canEat = config["ejected"]["canEat"];
    avoidSpawningOn = nothing; // Must be able to be spawned near any entity

    isSpiked = config["ejected"]["isSpiked"];
    isAgitated = config["ejected"]["isAgitated"];
}
Ejected::~Ejected() {
}

/********* MOTHERCELL *********/

MotherCell::MotherCell(const Vector2 &pos, double radius, const Color &color) noexcept :
    Entity(pos, radius, color) {
    type = CellType::MOTHERCELL;

    flag = mothercells;
    canEat = config["motherCell"]["canEat"];
    avoidSpawningOn = config["motherCell"]["avoidSpawningOn"];

    isSpiked = config["motherCell"]["isSpiked"];
    isAgitated = config["motherCell"]["isAgitated"];
}
void MotherCell::onRemove() {
    // Same applies for mothercells
    map::spawn<MotherCell>(randomPosition(), config["motherCell"]["baseRadius"], config["motherCell"]["color"]);
}
MotherCell::~MotherCell() {
}

/********* PLAYERCELL *********/

PlayerCell::PlayerCell(const Vector2 &pos, double radius, const Color &color) noexcept :
    Entity(pos, radius, color) {
    type = CellType::PLAYERCELL;

    flag = playercells;
    canEat = config["playerCell"]["canEat"];
    avoidSpawningOn = config["playerCell"]["avoidSpawningOn"];

    isSpiked = config["playerCell"]["isSpiked"];
    isAgitated = config["playerCell"]["isAgitated"];
}
void PlayerCell::move() {
    Vector2 d = owner->getMouse() - _position;

    double squared = d.squared();
    if (squared < 1) return;

    double distance = std::sqrt(squared);
    double invd = 1 / distance;

    // normalized distance (0..1)
    distance = std::min(distance, 32.0) / 32;

	// https://imgur.com/a/H9s0J
    double speed = (2.2 * std::pow(_radius, -0.4396754)) * (int)config["game"]["timeStep"];
    speed *= (int)config["playerCell"]["speedMultiplier"] * distance;

    if (speed > 0)
        _position += (d * invd) * speed;
}
void PlayerCell::update(unsigned long long tick) {
    // Update decay once per second
    if ((tick % 25) != 0) return;

    double rate = config["playerCell"]["radiusDecayRate"];
    if (rate <= 0) return;

    double newRadius = std::sqrt(radiusSquared() * rate);
    if (newRadius <= (double)config["playerCell"]["baseRadius"])
        return;

    setRadius(newRadius);
}

PlayerCell::~PlayerCell() {
}
