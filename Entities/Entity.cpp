#include "Entity.hpp"
#include "../Game/Map.hpp"
#include "../Game/Game.hpp" // configs

//************************* SETTERS *************************//

void Entity::setOwner(Player *owner) noexcept {
    _owner = owner;
}
void Entity::setColor(const Color &color) noexcept {
    _color = color;
    needsUpdate = true;
}
void Entity::setPosition(const Vec2 &position, bool validate) noexcept {
    needsUpdate = true;
    _position = position;

    if (!validate) return; // No need
    
    const double hr = _radius * 0.5f; // half radius

    // Validate left
    double maxIndent = map::bounds().left() + hr;
    if (_position.x <= maxIndent) {
        _position.x = maxIndent;
        _velocity.x = -_velocity.x;
    }
    // Validate right
    if ((maxIndent = map::bounds().right() - hr) < _position.x) {
        _position.x = maxIndent;
        _velocity.x = -_velocity.x;
    }
    // Validate bottom
    if ((maxIndent = map::bounds().bottom() + hr) > _position.y) {
        _position.y = maxIndent;
        _velocity.y = -_velocity.y;
    }
    // Validate top
    if ((maxIndent = map::bounds().top() - hr) < _position.y) {
        _position.y = maxIndent;
        _velocity.y = -_velocity.y;
    }
}
void Entity::setVelocity(double acceleration, double angle) noexcept {
    _acceleration = acceleration;
    _velocity = {
        std::cos(angle),
        std::sin(angle)
    };
    map::acceleratingEntities[_nodeId] = shared;
}
void Entity::setMass(double mass) noexcept {
    _mass = mass;
    _radius = toRadius(mass);
    needsUpdate = true;
}
void Entity::setRadius(double radius) noexcept {
    _radius = radius;
    _mass = toMass(radius);
    needsUpdate = true;
}
void Entity::setCreator(unsigned int id) noexcept {
    _creatorId = id;
}
void Entity::setBirthTick(Game *_game) noexcept {
    game = _game;
    birthTick = _game->tickCount;
}

//************************* GETTERS *************************//

Player *Entity::owner() const noexcept {
    return _owner;
}
const Color &Entity::color() const noexcept {
    return _color;
}
const Vec2 &Entity::position() const noexcept {
    return _position;
}
const Vec2 &Entity::velocity() const noexcept {
    return _velocity;
}
double Entity::mass() const noexcept {
    return _mass;
}
double Entity::radius() const noexcept {
    return _radius;
}
double Entity::acceleration() const noexcept {
    return _acceleration;
}
double Entity::radiusSquared() const noexcept {
    return _radius * _radius;
}
unsigned int Entity::nodeId() const noexcept {
    return _nodeId;
}
unsigned int Entity::creator() const noexcept {
    return _creatorId;
}
unsigned int Entity::killerId() const noexcept {
    return _killerId;
}
unsigned long long Entity::age() const noexcept {
    return game->tickCount - birthTick;
}

//************************* MISC *************************//

bool Entity::decelerate() noexcept {
    if (isRemoved) {
        //logVerbose(Entity could not be decelerated because it is removed.);
        _acceleration = 0;
        return false;
    }
    if (_acceleration <= 0.01) {
        //logVerbose(Entity could not be decelerated because its acceleration is too low.);
        _acceleration = 0;
        return false;
    }

    // decelerate by X units per tick
    double deceleration = _acceleration / cfg::entity_decelerationPerTick;
    _acceleration -= deceleration;
    
    // https://gist.github.com/Megabyte918/0b921e69f9d84b3ea7b8fdebef4f6812#file-gameconfiguration-json-L142
    if (_acceleration >= cfg::entity_minAcceleration)
        // Keep accelerating in the same direction
        setPosition(_position + _velocity * deceleration, true);
    return true;
}
bool Entity::intersects(e_ptr other) const noexcept {
    return intersects(other->_position, other->_radius);
}

bool Entity::intersects(const Vec2 &pos, double radius) const noexcept {
    double rs = _radius + radius;
    return (_position - pos).squared() < (rs * rs);
}
void Entity::move() noexcept {
}
void Entity::pop() noexcept {
}
void Entity::split(double angle, double radius) noexcept {
    angle, radius;
}
void Entity::autoSplit() noexcept {
}
void Entity::update(unsigned long long tick) noexcept {
    tick;
}
void Entity::onDespawned() const noexcept  {
}
void Entity::consume(e_ptr &prey) noexcept {
    // Not allowed to eat or is already removed
    if (!(canEat & prey->flag) || prey->isRemoved)
        return;

    // https://gist.github.com/Megabyte918/0b921e69f9d84b3ea7b8fdebef4f6812#file-gameconfiguration-json-L178
    // assuming "percentageOfCellToSquash" is the range required to eat another cell
    double range = _radius - cfg::entity_minEatOverlap * prey->_radius;
    if ((_position - prey->_position).squared() >= range * range)
        return; // Not close enough to eat

    prey->_killerId = _nodeId; // prey was killed by this
    setMass(_mass + prey->_mass); // add prey's mass to this
    map::despawn(prey); // remove prey from map
}
// debugging purposes
std::string Entity::toString() const noexcept {
    std::stringstream ss;

    ss << "type: " << type
        << "\nflag: " << +flag
        << "\ncanEat: " << +canEat
        << "\navoidSpawningOn: " << +avoidSpawningOn

        << "\n\nisSpiked: " << isSpiked
        << "\nisAgitated: " << isAgitated
        << "\nisRemoved: " << isRemoved
        << "\nneedsUpdate: " << needsUpdate

        << "\nobj: {\n"
        << "\n    data: " << std::any_cast<Entity*>(obj.data)
        << "\n    bound: {"
        << "\n        x():" << obj.bound.x()
        << "\n        y():" << obj.bound.y()
        << "\n        width():" << obj.bound.width()
        << "\n        height():" << obj.bound.height()
        << "\n        halfWidth():" << obj.bound.halfWidth()
        << "\n        halfHeight():" << obj.bound.halfHeight()
        << "\n        left():" << obj.bound.left()
        << "\n        top():" << obj.bound.top()
        << "\n        right():" << obj.bound.right()
        << "\n        bottom():" << obj.bound.bottom()
        << "\n    }"
        << "\n}"

        << "\nnodeId(): " << nodeId()
        << "\nkillerId(): " << killerId()
        << "\ncreator(): " << creator()
        << "\ngetOwner(): " << owner()
        << "\ngetPosition(): " << position().toString()
        << "\ngetColor(): " << const_cast<Color&>(color()).toString()
        << "\ngetMass(): " << mass()
        << "\ngetRadius(): " << radius()
        << "\nradiusSquared(): " << radiusSquared()
        << "\nage(): " << age()
        << "\nshared: {"
        << "\n    unique(): " << 0/*shared.unique()*/
        << "\n    use_count(): " << shared.use_count()
        << "\n}"
        << "\n_velocity: " << _velocity.toString()
        << "\n_acceleration: " << _acceleration
        << "\nbirthTick: " << birthTick
        << "\ngame: " << game;
         

    return ss.str();
}

Entity::Entity(const Vec2 &pos, double radius, const Color &color) noexcept {
    setPosition(pos);
    setRadius(radius);
    setColor(color);
    _nodeId = prevNodeId == 4294967295 ? 1 : ++prevNodeId;
}

Entity::~Entity() {
}