#pragma once

#include "EntityHandler.h"

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

        Entity();
        virtual ~Entity();
};

class Food: public Entity {
    public:
        Food();
        ~Food();
};

class Virus: public Entity {
    public:
        Virus();
        ~Virus();
};

class Ejected: public Entity {
    public:
        Ejected();
        ~Ejected();
};

class MotherCell: public Entity {
    public:
        MotherCell();
        ~MotherCell();
};

class PlayerCell: public Entity {
    public:
        PlayerCell();
        ~PlayerCell();
};
