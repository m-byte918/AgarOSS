#include "Ejected.hpp"
#include "PlayerCell.hpp"
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
    _position = position;

    // Bounce off of map borders
    if (validate) {
        // Validate left
        const float hr = _radius * 0.5f; // half radius
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
    obj.bound.setPosition(_position.x, _position.y);
    Entity::update();
}
void Entity::setVelocity(float acceleration, double angle) noexcept {
    _acceleration = acceleration;
    _velocity = {
        std::cos(angle),
        std::sin(angle)
    };
    map::movingEntities.push_back(shared);
}
void Entity::setVelocity(float acceleration, Vec2 velocity) noexcept {
    _acceleration = acceleration;
    _velocity = velocity;
    map::movingEntities.push_back(shared);
}
void Entity::setMass(float mass) noexcept {
    _mass = mass;
    _invMass = 1.0f / mass;
    _radius = toRadius(mass);
    obj.bound.setSize(_radius * 2.0, _radius * 2.0);
    Entity::update();
}
void Entity::setRadius(float radius) noexcept {
    _radius = radius;
    _mass = toMass(radius);
    _invMass = 1.0f / _mass;
    obj.bound.setSize(_radius * 2.0, _radius * 2.0);
    Entity::update();
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
float Entity::invMass() const noexcept {
    return _invMass;
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
    // decelerate by X units per tick
    float deceleration = std::round(_acceleration / cfg::entity_decelerationPerTick);
    if (deceleration <= cfg::entity_minAcceleration) {
        _acceleration = 0.0f;
        return false;
    }
    _acceleration -= deceleration;
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
}
void Entity::autoSplit() noexcept {
}
void Entity::update() noexcept {
    if (!map::quadTree.update(&obj) && game != nullptr) {
        Logger::error("Entity could not be updated: ", toString());
        // If removed from quadtree and not re-inserted for ANY reason, re-insert it.
        if (!map::quadTree.contains(&obj))
            map::quadTree.insert(&obj);
    } else {
        state |= needsUpdate;
    }
}
void Entity::onDespawned() noexcept  {
}
void Entity::collideWith(e_ptr other) noexcept {
    if (!shared || !other || state & isRemoved || other->state & isRemoved || !intersects(other))
        return;

    // Determine if predator should become prey
    bool isPredatorSmaller = _radius <= other->radius() * cfg::entity_minEatSizeMult;

    // Resolve rigid collisions
    if (type == other->type) {
        // Ejected -> resolve collision immediately
        if (type == Ejected::TYPE) {
            map::resolveCollision(shared, other);
            // Set velocity again to start chain reaction
            if (other->acceleration() == 0.0f)
                other->setVelocity(5.125f, position().angle());
            return;
        }
        // Playercells from same owner
        else if (_creatorId == other->creator()) {
            if (!(state & ignoreCollision) || !(other->state & ignoreCollision)) {
                // Just split -> resolve collision after 15 ticks
                if (age() > cfg::player_collisionIgnoreTime &&
                    other->age() > cfg::player_collisionIgnoreTime) {
                    map::resolveCollision(shared, other);
                    return;
                }
                return; // Merging -> do not eat or resolve collision
            }
        }
        // Viruses & Mothercells -> do not eat or resolve collision
        else if (type != PlayerCell::TYPE && _owner == other->owner())
            return;
        // Playercells from different owners -> do not consume
        // if predator is smaller
        else if (isPredatorSmaller)
            return;
    }
    // Resolve eat collisions
    e_ptr predator = shared;
    e_ptr prey = other;
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
void Entity::consume(e_ptr prey) noexcept {
    prey->setKiller(_nodeId); // prey was killed by this
    setMass(_mass + prey->_mass); // add prey's mass to this
    map::despawn(prey); // remove prey from map
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

        << "\nmouseCache: " << mouseCache.toString()
        << "\nspeedMultiplier: " << speedMultiplier

        << "\n\nshared: " << shared
        << " {\n    use_count(): " << shared.use_count()
        << "\n    get(): " << shared.get()
        << "\n}"
        << "\nobj: {"
        << "\n    data: " << (obj.data.has_value() ? std::any_cast<e_ptr>(obj.data) : 0)
        << "\n    bound: {"
        << "\n        x(): " << obj.bound.x()
        << "\n        y(): " << obj.bound.y()
        << "\n        width(): " << obj.bound.width()
        << "\n        height(): " << obj.bound.height()
        << "\n        halfWidth(): " << obj.bound.halfWidth()
        << "\n        halfHeight(): " << obj.bound.halfHeight()
        << "\n        left(): " << obj.bound.left()
        << "\n        top(): " << obj.bound.top()
        << "\n        right(): " << obj.bound.right()
        << "\n        bottom(): " << obj.bound.bottom()
        << "\n    }"
        << "\n}"
        << "\nowner(): " << owner()
        << "\ncolor(): " << const_cast<Color&>(color()).toString()
        << "\nposition(): " << position().toString()
        << "\nvelocity(): " << velocity().toString()
        << "\nmass(): " << mass()
        << "\nradius(): " << radius()
        << "\ninvMass(): " << invMass()
        << "\nacceleration(): " << acceleration()
        << "\nradiusSquared(): " << radiusSquared()
        << "\nnodeId(): " << nodeId()
        << "\ncreator(): " << creator()
        << "\nkillerId(): " << killerId()
        << "\nage(): " << age()
        << "\n\ndecelerate(): " << decelerate()

        << "\n\nbirthTick: " << birthTick
        << "\ngame: " << game
        << "\nis in quadtree? " << map::quadTree.contains(&obj)
        << "\nis in its vector? " << 
        (std::find(map::entities[type].begin(), map::entities[type].end(), shared) != map::entities[type].end());
        
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