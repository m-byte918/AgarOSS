#pragma once
#include "Entity.hpp"

class Food : public Entity {
public:
    static const int TYPE = 0;

    Food(const Vec2&, float radius, const Color&) noexcept;
    void update() noexcept;
    void onDespawned() noexcept;
    ~Food();
private:
    unsigned short growTick = 0;
};