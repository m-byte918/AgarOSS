#pragma once
#include "Entity.hpp"

class PlayerCell : public Entity {
public:
    static const int TYPE = 4;

    PlayerCell(const Vec2&, float radius, const Color&) noexcept;
    void move() noexcept;
    void autoSplit() noexcept;
    void pop() noexcept;
    void split(double angle, float radius) noexcept;
    void update() noexcept;
    void consume(e_ptr _prey) noexcept;
    void onDespawned() noexcept;
    ~PlayerCell();
private:
    unsigned char decayTick = 0;
};