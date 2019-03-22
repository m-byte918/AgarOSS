#pragma once
#include "Entity.hpp"

class Ejected : public Entity {
public:
    Ejected(const Vec2&, float radius, const Color&) noexcept;
    ~Ejected();
};