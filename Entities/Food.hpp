#pragma once
#include "Entity.hpp"

class Food : public Entity {
public:
    Food(const Vec2&, double radius, const Color&) noexcept;
    void update(unsigned long long tick) noexcept;
    void onDespawned() const noexcept;
    ~Food();
};