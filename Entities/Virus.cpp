#include "Virus.hpp"
#include "../Game/Map.hpp"
#include "../Game/Game.hpp" // configs

Virus::Virus(const Vector2 &pos, double radius, const Color &color) noexcept :
    Entity(pos, radius, color) {
    type = CellType::VIRUS;

    flag = viruses;
    canEat = cfg::virus_canEat;
    avoidSpawningOn = cfg::virus_avoidSpawningOn;

    isSpiked = cfg::virus_isSpiked;
    isAgitated = cfg::virus_isAgitated;
}
void Virus::split(double angle) noexcept {
    // Reset radius from splitting virus
    setRadius(cfg::virus_baseRadius);

    // Spawn new virus at splitting virus's position with base virus radius
    e_ptr newCell = map::spawn<Virus>(_position, cfg::virus_baseRadius, _color);
    newCell->setVelocity(cfg::virus_initialAcceleration, angle);
    newCell->setCreator(_nodeId);
}
void Virus::onDespawned() const noexcept {
    // Same thing applies to viruses
    if (map::entities[CellType::VIRUS].size() < cfg::virus_maxAmount)
        map::spawn<Virus>(randomPosition(), cfg::virus_baseRadius, cfg::virus_color);
}
void Virus::consume(const e_ptr &prey) noexcept {
    Entity::consume(prey);

    if (_radius >= cfg::virus_maxRadius)
        split(prey->velocity.angle());
}
Virus::~Virus() {
}