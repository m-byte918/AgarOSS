#include "Entity.hpp"
#include <sstream> // Entity::toString()
#include "../Game/Map.hpp"
#include "../Game/Game.hpp" // configs

//************************* SETTERS *************************//

void Entity::setOwner(Player *owner) noexcept {
    _owner = owner;
}
void Entity::setColor(const Color &color) noexcept {
    _color = color;
    state |= needsUpdate;
}
void Entity::setPosition(const Vec2 &position, bool validate) noexcept {
    state |= needsUpdate;
    _position = position;

    if (!validate) return; // No need
    
    const float hr = _radius * 0.5f; // half radius

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
void Entity::setVelocity(float acceleration, double angle) noexcept {
    _acceleration = acceleration;
    _velocity = {
        std::cos(angle),
        std::sin(angle)
    };
    map::acceleratingEntities[_nodeId] = shared;
}
void Entity::setMass(float mass) noexcept {
    _mass = mass;
    _radius = toRadius(mass);
    state |= needsUpdate;
}
void Entity::setRadius(float radius) noexcept {
    _radius = radius;
    _mass = toMass(radius);
    state |= needsUpdate;
}
void Entity::setCreator(unsigned int id) noexcept {
    _creatorId = id;
}
void Entity::setKiller(unsigned int id) noexcept {
    _killerId = id;
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
float Entity::mass() const noexcept {
    return _mass;
}
float Entity::radius() const noexcept {
    return _radius;
}
float Entity::acceleration() const noexcept {
    return _acceleration;
}
float Entity::radiusSquared() const noexcept {
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
    if (state & isRemoved) {
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
    float deceleration = _acceleration / cfg::entity_decelerationPerTick;
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

bool Entity::intersects(const Vec2 &pos, float radius) const noexcept {
    float rs = _radius + radius;
    return (_position - pos).squared() < (rs * rs);
}
void Entity::move() noexcept {
}
void Entity::pop() noexcept {
}
void Entity::split(double angle, float radius) noexcept {
    angle, radius;
}
void Entity::autoSplit() noexcept {
}
void Entity::update(unsigned long long tick) noexcept {
    tick;
}
void Entity::onDespawned() noexcept  {
}
void Entity::collideWith(Entity *other) noexcept {
    if (!shared || !other->shared || state & isRemoved || 
        other->state & isRemoved || !intersects(other->shared))
        return;

    // Determine if predator should become prey
    bool isPredatorSmaller = _radius <= other->radius() * cfg::entity_minEatSizeMult;

    // Resolve rigid collisions
    if (_creatorId != 0 && other->creator() != 0 && type == other->type) {
        // Ejected -> resolve collision immediately
        if (type == CellType::EJECTED) {
            map::resolveCollision(shared, other->shared);
            // Set velocity again to start chain reaction
            if (other->acceleration() == 0)
                other->setVelocity(1.0f, position().angle());
            return;
        }
        // Playercells from same owner
        else if (_creatorId == other->creator()) {
            if (!(state & ignoreCollision) || !(other->state & ignoreCollision)) {
                // Just split -> resolve collision after 15 ticks
                if (age() > cfg::player_collisionIgnoreTime &&
                    other->age() > cfg::player_collisionIgnoreTime) {
                    map::resolveCollision(shared, other->shared);
                    return;
                }
                return; // Merging -> do not eat or resolve collision
            }
        }
        // Viruses & Mothercells -> do not eat or resolve collision
        else if (type != CellType::PLAYERCELL && _owner == other->owner())
            return;
        // Playercells from different owners -> do not consume
        // if predator is smaller
        else if (isPredatorSmaller)
            return;
    }
    // Resolve eat collisions
    e_ptr predator = shared;
    e_ptr prey = other->shared;
    if (isPredatorSmaller) {
        predator = prey;
        prey = shared;
    }
    // Not allowed to eat or is already removed
    if (!(predator->canEat & prey->flag) || prey->state & isRemoved)
        return;
    // https://gist.github.com/Megabyte918/0b921e69f9d84b3ea7b8fdebef4f6812#file-gameconfiguration-json-L178
    // assuming "percentageOfCellToSquash" is the range required to eat another cell
    float range = predator->_radius - cfg::entity_minEatOverlap * prey->_radius;
    if ((predator->_position - prey->_position).squared() >= range * range)
        return; // Not close enough to eat
    predator->consume(prey);
}
void Entity::consume(e_ptr &prey) noexcept {
    prey->setKiller(_nodeId); // prey was killed by this
    setMass(_mass + prey->_mass); // add prey's mass to this
    map::despawn(prey.get()); // remove prey from map
}
// debugging purposes
std::string Entity::toString() noexcept {
    std::stringstream ss;

    ss << "type: " << type
        << "\nflag: " << +flag
        << "\ncanEat: " << +canEat
        << "\navoidSpawningOn: " << +avoidSpawningOn

        << "\nisSpiked: " << (state & isSpiked)
        << "\nisAgitated: " << (state & isAgitated)
        << "\nisRemoved: " << (state & isRemoved)
        << "\nneedsUpdate: " << (state & needsUpdate)
        << "\nignoreCollision: " << (state & ignoreCollision)

        << "\n\nmouseCache: " << mouseCache.toString()
        << "\ncellAmountCache: " << cellAmountCache
        << "\nspeedMultiplier: " << speedMultiplier

        << "\n\nshared: {"
        << "\n    unique(): " << 0/*shared.unique()*/
        << "\n    use_count(): " << shared.use_count()
        << "\n}"

        << "\n\nobj: {\n"
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

        << "\n\nowner(): " << owner()
        << "\ncolor(): " << const_cast<Color&>(color()).toString()
        << "\nposition(): " << position().toString()
        << "\nvelocity(): " << velocity().toString()
        << "\nmass(): " << mass()
        << "\nradius(): " << radius()
        << "\nacceleration(): " << acceleration()
        << "\nradiusSquared(): " << radiusSquared()
        << "\nnodeId(): " << nodeId()
        << "\ncreator(): " << creator()
        << "\nkillerId(): " << killerId()
        << "\nage(): " << age()

        << "\n\ndecelerate(): " << decelerate()

        << "\n\nbirthTick: " << birthTick
        << "\ngame: " << game;
    return ss.str();
}

Entity::Entity(const Vec2 &pos, float radius, const Color &color) noexcept {
    setPosition(pos);
    setRadius(radius);
    setColor(color);
    _nodeId = prevNodeId == 4294967295 ? 1 : ++prevNodeId;
}

Entity::~Entity() {
}