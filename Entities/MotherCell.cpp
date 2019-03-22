#include "MotherCell.hpp"
#include "../Game/Map.hpp"
#include "../Game/Game.hpp" // configs

MotherCell::MotherCell(const Vec2 &pos, float radius, const Color &color) noexcept :
    Entity(pos, radius, color) {
    type = CellType::MOTHERCELL;

    flag = mothercells;
    canEat = cfg::motherCell_canEat;
    avoidSpawningOn = cfg::motherCell_avoidSpawningOn;

    if (cfg::motherCell_isSpiked)   state |= isSpiked;
    if (cfg::motherCell_isAgitated) state |= isAgitated;
}
void MotherCell::onDespawned() noexcept {
    // Spawn a new one immediately
    if (map::entities[CellType::MOTHERCELL].size() < cfg::motherCell_startAmount)
        map::spawn<MotherCell>(randomPosition(), cfg::motherCell_baseRadius, cfg::motherCell_color);
}
MotherCell::~MotherCell() {
}