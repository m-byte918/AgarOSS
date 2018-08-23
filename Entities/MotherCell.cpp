#include "MotherCell.hpp"
#include "../Game/Map.hpp"
#include "../Game/Game.hpp" // configs

MotherCell::MotherCell(const Vector2 &pos, double radius, const Color &color) noexcept :
    Entity(pos, radius, color) {
    type = CellType::MOTHERCELL;

    flag = mothercells;
    canEat = cfg::motherCell_canEat;
    avoidSpawningOn = cfg::motherCell_avoidSpawningOn;

    isSpiked = cfg::motherCell_isSpiked;
    isAgitated = cfg::motherCell_isAgitated;
}
void MotherCell::onDespawned() const noexcept {
    // Spawn a new one immediately
    if (map::entities[CellType::MOTHERCELL].size() < cfg::motherCell_maxAmount)
        map::spawn<MotherCell>(randomPosition(), cfg::motherCell_baseRadius, cfg::motherCell_color);
}
MotherCell::~MotherCell() {
}