#pragma once
#include "EntityHandler.h"

using namespace EntityHandler; // careful with this

class Entity {
private:
    double _mass;
    double _size;
    double _sizeSquared;
    Color    _color;
    Position _position;
    const unsigned int _nodeId = ++prevNodeId;

public:
    const unsigned int &getNodeId() const;

    void setPosition(const Position &position);
    const Position &getPosition() const;

    void setColor(const Color &color);
    const Color &getColor() const;

    void setSize(const double &size);
    const double &getSize() const;
    const double &getSizeSquared() const;
    const double &getMass() const;

    Entity(const double &size, const Color &color, const Position &position);
    Entity();
    virtual ~Entity();
};

class Food: public Entity {
    using Entity::Entity;
public:
    Food();
    ~Food();
};

class Virus: public Entity {
    using Entity::Entity;
public:
    Virus();
    ~Virus();
};

class Ejected: public Entity {
    using Entity::Entity;
public:
    Ejected(const Color &color, const Position &position);
    ~Ejected();
};

class MotherCell: public Entity {
    using Entity::Entity;
public:
    MotherCell();
    ~MotherCell();
};

class PlayerCell: public Entity {
    using Entity::Entity;
public:
    PlayerCell();
    ~PlayerCell();
};
