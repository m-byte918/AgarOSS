#include "Ejected.hpp"
#include "../Game/Game.hpp" // configs

Ejected::Ejected(const Vector2 &pos, double radius, const Color &color) noexcept :
    Entity(pos, radius, color) {
    type = CellType::EJECTED;

    flag = ejected;
    canEat = cfg::ejected_canEat;
    avoidSpawningOn = nothing; // Must be able to be spawned near any entity

    isSpiked = cfg::ejected_isSpiked;
    isAgitated = cfg::ejected_isAgitated;
}

Ejected::~Ejected() {
}