#pragma once
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
    int           type            = -1; // CellType this entity is classified as
    unsigned char flag            = nothing; // Group this entity belongs to
    unsigned char canEat          = nothing; // Cell types this entity can eat
    unsigned char avoidSpawningOn = nothing; // Cell types to be checked for safe spawn

    // States
    bool isSpiked        = false; // Cell has spikes on its outline
    bool isAgitated      = false; // Cell has waves on its outline
    bool isRemoved       = false; // Cell was removed from map
    bool needsUpdate     = true;  // Cell needs updating on client side
    bool ignoreCollision = false; // Whether or not to ignore collision with self

    // Miscc
    e_ptr shared; // Shared pointer for this entity
    Collidable obj; // Object to insert into quadTree

    // Setters
    void setOwner(Player *owner) noexcept;
    void setColor(const Color &color) noexcept;
    void setPosition(const Vec2 &position, bool validate = false) noexcept;
    void setVelocity(double velocity, double angle) noexcept;
    void setMass(double mass) noexcept;
    void setRadius(double radius) noexcept;
    void setCreator(unsigned int id) noexcept;
    void setBirthTick(Game *_game) noexcept;

    // Getters
    Player *owner() const noexcept;
    const Color &color() const noexcept;
    const Vec2 &position() const noexcept;
    const Vec2 &velocity() const noexcept;
    double mass() const noexcept;
    double radius() const noexcept;
    double acceleration() const noexcept;
    double radiusSquared() const noexcept;
    unsigned int nodeId() const noexcept;
    unsigned int creator() const noexcept;
    unsigned int killerId() const noexcept;
    unsigned long long age() const noexcept;

    // Misc
    bool decelerate() noexcept;
    bool intersects(e_ptr other) const noexcept;
    bool intersects(const Vec2 &pos, double radius) const noexcept;
    virtual void move() noexcept;
    virtual void pop() noexcept;
    virtual void split(double angle, double radius) noexcept;
    virtual void autoSplit() noexcept;
    virtual void update(unsigned long long tick) noexcept;
    virtual void onDespawned() const noexcept;
    virtual void consume(e_ptr &_prey) noexcept;
    std::string toString() const noexcept;

    Entity(const Vec2&, double radius, const Color&) noexcept;
    virtual ~Entity();

protected:
    Color _color{ 0, 0, 0 };
    Vec2  _velocity{ 1, 0 };
    Vec2  _position{ 0, 0 };

    double _mass         = 0;
    double _radius       = 0;
    double _acceleration = 0;

    unsigned int      _nodeId    = 0;
    unsigned int      _killerId  = 0;
    unsigned int      _creatorId = 0;
    unsigned long long birthTick = 0;
    
    Game   *game   = nullptr; // Game this entity belongs to
    Player *_owner = nullptr; // Player that owns this cell
};