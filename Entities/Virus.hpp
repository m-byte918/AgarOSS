#pragma once
#include "Entity.hpp"

class Virus : public Entity {
public:
    Virus(const Vector2&, double radius, const Color&) noexcept;
    void split(double angle) noexcept;
    void onDespawned() const noexcept;
    void consume(const e_ptr &prey) noexcept;
    ~Virus();
};