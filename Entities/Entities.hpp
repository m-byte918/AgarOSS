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
    unsigned int prevNodeId = 0;
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
    Player *owner = nullptr; // Player that owns this cell

    const unsigned int &nodeId() const noexcept;
    const unsigned int &killerId() const noexcept;

    virtual void setPosition(const Vector2 &position) noexcept;
    const Vector2 &getPosition() const noexcept;

    void setColor(const Color &color) noexcept;
    const Color &getColor() const noexcept;

    void setMass(double mass) noexcept;
    double getMass() const noexcept;

    void setRadius(double radius) noexcept;
    double getRadius() const noexcept;

    double radiusSquared() const noexcept;

    virtual void move();
    virtual void update(unsigned long long tick);
    virtual void onRemove();

    void consume(Entity *prey);
    bool intersects(Entity *other) const noexcept;
    bool intersects(const Vector2 &pos, double radius) const noexcept;

    Entity(const Vector2&, double radius, const Color&) noexcept;
    virtual ~Entity();

private:
    const unsigned int _nodeId = ++prevNodeId;
    unsigned int _killerId = 0;

protected:
    double  _mass = 0;
    double  _radius = 0;
    Color   _color{ 0, 0, 0 };
    Vector2 _position{ 0, 0 };
};

class Food : public Entity {
public:
    Food(const Vector2&, double radius, const Color&) noexcept;
    void update(unsigned long long tick);
    void onRemove();
    ~Food();

private:
    bool canGrow = config["food"]["canGrow"];
    double maxRadius = config["food"]["maxRadius"];
};

class Virus : public Entity {
public:
    Virus(const Vector2&, double radius, const Color&) noexcept;
    void onRemove();
    ~Virus();
};

class Ejected : public Entity {
public:
    Ejected(const Vector2&, double radius, const Color&) noexcept;
    ~Ejected();
};

class MotherCell : public Entity {
public:
    MotherCell(const Vector2&, double radius, const Color&) noexcept;
    void onRemove();
    ~MotherCell();
};

class PlayerCell : public Entity {
public:
    PlayerCell(const Vector2&, double radius, const Color&) noexcept;
    void move();
    void update(unsigned long long tick);
    ~PlayerCell();
};