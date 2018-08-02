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
    int cellType;
    Collidable obj;
    bool isSpiked    = false; // Cell has spiked on its outline
    bool isAgitated  = false; // Cell has waves on its outline
    bool isRemoved   = false; // Cell was removed from map
    bool needsUpdate = false; // Cell needs updating on client side
    Player *owner = nullptr;

    const unsigned int &getNodeId() const noexcept;
    const unsigned int &getKillerId() const noexcept;

    void setPosition(const Vector2 &position) noexcept;
    const Vector2 &getPosition() const noexcept;

    void setColor(const Color &color) noexcept;
    const Color &getColor() const noexcept;

    void setSize(double _size) noexcept;
    double getSize() const noexcept;
    double getSizeSquared() const noexcept;
    double getMass() const noexcept;

    virtual void move();
    virtual void onRemove();
    virtual void updateDecay();
    void consume(Entity *other);
    bool intersects(Entity *other) const noexcept;
    bool intersects(const Vector2 &pos, double _size) const noexcept;

    Entity(const Vector2&, double _size, const Color&) noexcept;
    virtual ~Entity();

private:
    double mass = 0;
    Color color;
    const unsigned int nodeId = ++prevNodeId;
    unsigned int killerId = 0;

protected:
    Vector2 position;

    double size = 0;
    double sizeSquared = 0;
};

class Food : public Entity {
    using Entity::Entity;
public:
    Food(const Vector2&, double _size, const Color&) noexcept;
    void onRemove();
    ~Food();
};

class Virus : public Entity {
    using Entity::Entity;
public:
    Virus(const Vector2&, double _size, const Color&) noexcept;
    void onRemove();
    ~Virus();
};

class Ejected : public Entity {
    using Entity::Entity;
public:
    Ejected(const Vector2&, double _size, const Color&) noexcept;
    ~Ejected();
};

class MotherCell : public Entity {
    using Entity::Entity;
public:
    MotherCell(const Vector2&, double _size, const Color&) noexcept;
    void onRemove();
    ~MotherCell();
};

class PlayerCell : public Entity {
    using Entity::Entity;
public:
    PlayerCell(const Vector2&, double _size, const Color&) noexcept;
    void setSize(double _size) noexcept;
    void move();
    void updateDecay();
    ~PlayerCell();
};