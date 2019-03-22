#pragma once
#include "../Game/Game.hpp"
#include "../Modules/Utils.hpp"
#include "../Modules/QuadTree.hpp"

namespace {
    // Using struct instead of enum for use with vector
    // index(s) without having to type cast each time
    struct CellType {
        static const int FOOD = 0;
        static const int VIRUS = 1;
        static const int EJECTED = 2;
        static const int MOTHERCELL = 3;
        static const int PLAYERCELL = 4;
    };
    unsigned int prevNodeId = 0;
}
class Game;
class Player;

class Entity {
public:
    // Typing
    int           type            = -1;      // CellType this entity is classified as
    unsigned char flag            = nothing; // Group this entity belongs to
    unsigned char canEat          = nothing; // Cell types this entity can eat
    unsigned char avoidSpawningOn = nothing; // Cell types to be checked for safe spawn

    // States
    unsigned char state = 0x00;

    // Cached (for when owner disconnects)
    Vec2   mouseCache { 0, 0 };
    size_t cellAmountCache = 1;

    // Miscc
    e_ptr shared; // Shared pointer for this entity
    Collidable obj; // Object to insert into quadTree
    unsigned int speedMultiplier = cfg::playerCell_speedMultiplier; // Movement

    // Setters
    void setOwner(Player *owner) noexcept;
    void setColor(const Color &color) noexcept;
    void setPosition(const Vec2 &position, bool validate = false) noexcept;
    void setVelocity(float velocity, double angle) noexcept;
    void setMass(float mass) noexcept;
    void setRadius(float radius) noexcept;
    void setCreator(unsigned int id) noexcept;
    void setKiller(unsigned int id) noexcept;
    void setBirthTick(Game *_game) noexcept;

    // Getters
    Player *owner() const noexcept;
    const Color &color() const noexcept;
    const Vec2 &position() const noexcept;
    const Vec2 &velocity() const noexcept;
    float mass() const noexcept;
    float radius() const noexcept;
    float acceleration() const noexcept;
    float radiusSquared() const noexcept;
    unsigned int nodeId() const noexcept;
    unsigned int creator() const noexcept;
    unsigned int killerId() const noexcept;
    unsigned long long age() const noexcept;

    // Misc
    bool decelerate() noexcept;
    bool intersects(e_ptr other) const noexcept;
    bool intersects(const Vec2 &pos, float radius) const noexcept;
    virtual void move() noexcept;
    virtual void pop() noexcept;
    virtual void split(double angle, float radius) noexcept;
    virtual void autoSplit() noexcept;
    virtual void update(unsigned long long tick) noexcept;
    virtual void onDespawned() noexcept;
    virtual void collideWith(Entity *other) noexcept;
    virtual void consume(e_ptr &_prey) noexcept;
    std::string toString() noexcept;

    Entity(const Vec2&, float radius, const Color&) noexcept;
    virtual ~Entity();

protected:
    Color _color{ 0, 0, 0 };
    Vec2  _velocity{ 1, 0 };
    Vec2  _position{ 0, 0 };

    float _mass         = 0;
    float _radius       = 0;
    float _acceleration = 0;

    unsigned int      _nodeId    = 0;
    unsigned int      _killerId  = 0;
    unsigned int      _creatorId = 0;
    unsigned long long birthTick = 0;
    
    Game   *game   = nullptr; // Game this entity belongs to
    Player *_owner = nullptr; // Player that owns this cell
};