#pragma once
#include "Entity.hpp"

class MotherCell : public Entity {
public:
    MotherCell(const Vector2&, double radius, const Color&) noexcept;
    void onDespawned() const noexcept;
    ~MotherCell();
};