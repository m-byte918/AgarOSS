#include "Ejected.hpp"
#include "../Game/Game.hpp" // configs

Ejected::Ejected(const Vec2 &pos, float radius, const Color &color) noexcept :
    Entity(pos, radius, color) {

    flag = ejected;
    canEat = cfg::ejected_canEat;
    avoidSpawningOn = nothing; // Must be able to be spawned near any entity

    if (cfg::ejected_isSpiked)   state |= isSpiked;
    if (cfg::ejected_isAgitated) state |= isAgitated;
}

Ejected::~Ejected() {
}