#include "Entity.hpp"
#include "../Game/Map.hpp"
#include "../Game/Game.hpp" // configs

unsigned long long Entity::nodeId() const noexcept {
    return _nodeId;
}
unsigned long long Entity::killerId() const noexcept {
    return _killerId;
}

void Entity::setCreator(unsigned long long id) noexcept {
    _creatorId = id;
}
unsigned long long Entity::getCreator() const noexcept {
    return _creatorId;
}

void Entity::setOwner(Player *owner) noexcept {
    _owner = owner;
}
Player *Entity::getOwner() const noexcept {
    return _owner;
}

void Entity::setPosition(const Vector2 &position, bool validate) noexcept {
    needsUpdate = true;
    _position = position;

    if (!validate) return; // No need
    
    const Rect &bounds = map::getBounds(); // alias
    const double hr = _radius * 0.5f; // half radius
                            
    // Validate left
    double maxIndent = bounds.left() + hr;
    if (_position.x <= maxIndent) {
        _position.x = maxIndent;
        velocity.x = -velocity.x;
    }
    // Validate right
    maxIndent = bounds.right() - hr;
    if (_position.x >= maxIndent) {
        _position.x = maxIndent;
        velocity.x = -velocity.x;
    }
    // Validate bottom
    maxIndent = bounds.bottom() + hr;
    if (_position.y <= maxIndent) {
        _position.y = maxIndent;
        velocity.y = -velocity.y;
    }
    // Validate top
    maxIndent = bounds.top() - hr;
    if (_position.y >= maxIndent) {
        _position.y = maxIndent;
        velocity.y = -velocity.y;
    }
}
const Vector2 &Entity::getPosition() const noexcept {
    return _position;
}

void Entity::setColor(const Color &color) noexcept {
    _color = color;
    needsUpdate = true;
}
const Color &Entity::getColor() const noexcept {
    return _color;
}

void Entity::setMass(double mass) noexcept {
    _mass = mass;
    _radius = toRadius(mass);
    needsUpdate = true;
}
double Entity::getMass() const noexcept {
    return _mass;
}

void Entity::setRadius(double radius) noexcept {
    _radius = radius;
    _mass = toMass(radius);
    needsUpdate = true;
}
double Entity::getRadius() const noexcept {
    return _radius;
}

double Entity::radiusSquared() const noexcept {
    return _radius * _radius;
}

void Entity::setVelocity(double acceleration, double angle) noexcept {
    _acceleration = acceleration;
    velocity = { 
        std::cos(angle), 
        std::sin(angle)
    };
}
void Entity::decelerate() noexcept {
    if (isRemoved) return;

    // decelerate by X units per tick
    double deceleration = _acceleration / cfg::entity_decelerationPerTick;
    _acceleration -= deceleration;
    
    // Keep accelerating in the same direction
    setPosition(_position + velocity * deceleration, true);
}

// https://gist.github.com/Megabyte918/0b921e69f9d84b3ea7b8fdebef4f6812#file-gameconfiguration-json-L142
bool Entity::isAccelerating() const noexcept {
    return _acceleration > cfg::entity_minAcceleration;
}

void Entity::move() noexcept {
}
void Entity::split(double angle) noexcept {
}
void Entity::update(unsigned long long tick) noexcept {
}
void Entity::onDespawned() const noexcept  {
}

void Entity::consume(const e_ptr &prey) noexcept {
    // Not allowed to eat
    if (!(canEat & prey->flag)) return;

    // https://gist.github.com/Megabyte918/0b921e69f9d84b3ea7b8fdebef4f6812#file-gameconfiguration-json-L178
    // assuming "percentageOfCellToSquash" is the range required to eat another cell
    double range = _radius - cfg::entity_minEatOverlap * prey->_radius;
    if ((_position - prey->_position).squared() >= range * range)
        return; // Not close enough to eat

    prey->_killerId = _nodeId; // prey was killed by this
    setMass(_mass + prey->_mass); // add prey's mass to this
    map::despawn(prey); // remove prey from map
}
bool Entity::intersects(const e_ptr &other) const noexcept {
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
    _nodeId = prevNodeId == 4294967295 ? 1 : ++prevNodeId;
}

Entity::~Entity() {
}