#pragma once
#include "../Modules/Utils.h"

class Entity {
public:
    // Getters
    const int getId() const;
    const float getSize() const;
    const float getMass() const;
    const Color getColor() const;
    const Position getPosition() const;
    const float getSquareSize() const;

    // Setters
    void setSize(const float &size);
    void setPosition(const Position& position);
    void setColor(const Color& color);

    // Others
    bool canEat(const auto &entity) {
        return std::hypot(
        _position.x - entity->getPosition().x, 
        _position.y - entity->getPosition().y) 
        >= (_size - entity->getSize() / 3);
    }
    void operator<<(const float& size);
    Entity() {};
    virtual ~Entity() {};
private:
    Position _position;                 // Entity coordinates
    Color    _color;                    // Entity color
    int      _id = getNextNodeId();     // Entity ID
    float    _size, _mass, _squareSize; // Entity size, mass and squared size
};

class Food: public Entity {
public:
    Food() {};
    ~Food() {};
};

class Virus: public Entity {
public:
    Virus() {};
    ~Virus() {};
};

class Ejected: public Entity {
public:
    Ejected() {};
    ~Ejected() {};
};

class MotherCell: public Entity {
public:
    MotherCell() {};
    ~MotherCell() {};
};

class PlayerCell: public Entity {
public:
    PlayerCell() {};
    ~PlayerCell() {};
};
