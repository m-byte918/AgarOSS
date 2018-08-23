#pragma once
#include "Entity.hpp"

class Ejected : public Entity {
public:
    Ejected(const Vector2&, double radius, const Color&) noexcept;
    ~Ejected();
};