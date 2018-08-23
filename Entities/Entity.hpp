#pragma once
#include "../Modules/Utils.hpp"
#include "../Modules/QuadTree.hpp"

namespace {
    // using struct instead of enum for use with vector
    // index(s) without having to type cast each time
    struct CellType {
        static const int FOOD = 0;
        static const int VIRUS = 1;
        static const int EJECTED = 2;
        static const int MOTHERCELL = 3;
        static const int PLAYERCELL = 4;
    };
    unsigned long long prevNodeId = 0;
}

class Player;
class Entity {
public:
    int type = -1; // CellType this entity is classified as
    unsigned char flag = nothing; // Group this entity belongs to
    unsigned char canEat = nothing; // Cell types this entity can eat
    unsigned char avoidSpawningOn = nothing; // Cell types to be checked for safe spawn

    bool isSpiked    = false; // Cell has spiked on its outline
    bool isAgitated  = false; // Cell has waves on its outline
    bool isRemoved   = false; // Cell was removed from map
    bool needsUpdate = false; // Cell needs updating on client side

    Collidable obj; // Object to be inserted into quadTree

    unsigned long long nodeId() const noexcept;
    unsigned long long killerId() const noexcept;

    void setCreator(unsigned long long id) noexcept;
    unsigned long long getCreator() const noexcept;

    void setOwner(Player *owner) noexcept;
    Player *getOwner() const noexcept;

    virtual void setPosition(const Vector2 &position, bool validate = false) noexcept;
    const Vector2 &getPosition() const noexcept;

    void setColor(const Color &color) noexcept;
    const Color &getColor() const noexcept;

    void setMass(double mass) noexcept;
    double getMass() const noexcept;

    void setRadius(double radius) noexcept;
    double getRadius() const noexcept;

    double radiusSquared() const noexcept;

    void setVelocity(double velocity, double angle) noexcept;
    void decelerate() noexcept;

    bool isAccelerating() const noexcept;

    virtual void move() noexcept;
    virtual void split(double angle) noexcept;
    virtual void update(unsigned long long tick) noexcept;
    virtual void onDespawned() const noexcept;
    virtual void consume(const e_ptr &prey) noexcept;

    bool intersects(const e_ptr &other) const noexcept;
    bool intersects(const Vector2 &pos, double radius) const noexcept;

    Entity(const Vector2&, double radius, const Color&) noexcept;
    virtual ~Entity();

    Vector2 velocity{ 1, 0 };
protected:
    double  _mass = 0;
    double  _radius = 0;
    double  _acceleration = 0;
    Color   _color{ 0, 0, 0 };
    Vector2 _position{ 0, 0 };

    unsigned long long _nodeId = 0;
    unsigned long long _killerId = 0;
    unsigned long long _creatorId = 0;
    
    Player *_owner = nullptr; // Player that owns this cell
};