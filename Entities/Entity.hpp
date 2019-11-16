#pragma once
#include "../Game/Game.hpp"
#include "../Modules/Utils.hpp"
#include "../Modules/QuadTree.hpp"

namespace {
    struct Contact {
        Contact(e_ptr _A, e_ptr _B):
            A(_A), B(_B) {
        }
        e_ptr A = nullptr;
        e_ptr B = nullptr;
        Vec2 normal{ 1, 0 };
        double impulse = 0.0f;
        double penetration = 0.0f;
    };
    unsigned int prevNodeId = 0;
}
class Game;
class Player;

class Entity {
public:
    // Typing
    static const int TYPE            = -1;      // CellType the entity is classified as
    int              type            = -1;      // CellType this entity is classified as (rename)
    unsigned char    flag            = nothing; // Group this entity belongs to
    unsigned char    canEat          = nothing; // Cell types this entity can eat
    unsigned char    avoidSpawningOn = nothing; // Cell types to be checked for safe spawn

    // States
    unsigned char state = 0x00;

    // Cached (for when owner disconnects)
    Vec2 mouseCache{ 0, 0 };
    unsigned int speedMultiplier = cfg::playerCell_speedMultiplier;

    // Miscc
    e_ptr shared; // Shared pointer for this entity
    Collidable obj; // Object to insert into quadTree

    // Setters
    void setOwner(Player *owner) noexcept;
    void setColor(const Color &color) noexcept;
    void setPosition(const Vec2 &position, bool validate = false) noexcept;
    void setVelocity(float acceleration, double angle) noexcept;
    void setVelocity(float acceleration, Vec2 velocity) noexcept;
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
    float invMass() const noexcept;
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
    virtual void update() noexcept;
    virtual void onDespawned() noexcept;
    virtual void collideWith(e_ptr other) noexcept;
    virtual void consume(e_ptr _prey) noexcept;
    std::string toString() noexcept;

    Entity(const Vec2&, float radius, const Color&) noexcept;
    virtual ~Entity();

protected:
    Color _color{ 0, 0, 0 };
    Vec2  _position{ 0, 0 };
    Vec2  _velocity{ 1, 0 };

    float _mass         = 0.0f;
    float _radius       = 0.0f;
    float _invMass      = 0.0f;
    float _acceleration = 0.0f;

    unsigned int      _nodeId    = 0;
    unsigned int      _killerId  = 0;
    unsigned int      _creatorId = 0;
    unsigned long long birthTick = 0;
    
    Game   *game   = nullptr; // Game this entity belongs to
    Player *_owner = nullptr; // Player that owns this cell
};