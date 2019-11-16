#pragma once
#include "Entity.hpp"

class Ejected : public Entity {
public:
    static const int TYPE = 2;

    Ejected(const Vec2&, float radius, const Color&) noexcept;
    ~Ejected();
};