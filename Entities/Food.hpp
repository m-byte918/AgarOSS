#pragma once
#include "Entity.hpp"

class Food : public Entity {
public:
    Food(const Vector2&, double radius, const Color&) noexcept;
    void update(unsigned long long tick) noexcept;
    void onDespawned() const noexcept;
    ~Food();
};