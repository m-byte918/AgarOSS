#pragma once
#include "Entity.hpp"

class Virus : public Entity {
public:
    Virus(const Vec2&, double radius, const Color&) noexcept;
    void split(double angle, double radius) noexcept;
    void onDespawned() const noexcept;
    void consume(e_ptr &prey) noexcept;
    ~Virus();
};