#pragma once
#include "Entity.hpp"

class MotherCell : public Entity {
public:
    MotherCell(const Vec2&, float radius, const Color&) noexcept;
    void onDespawned() noexcept;
    ~MotherCell();
};