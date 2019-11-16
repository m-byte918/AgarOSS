#pragma once
#include "Entity.hpp"

class MotherCell : public Entity {
public:
    static const int TYPE = 3;
    MotherCell(const Vec2&, float radius, const Color&) noexcept;
    void onDespawned() noexcept;
    ~MotherCell();
};