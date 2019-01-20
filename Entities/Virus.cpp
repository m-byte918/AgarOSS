#include "Virus.hpp"
#include "../Game/Map.hpp"
#include "../Game/Game.hpp" // configs

Virus::Virus(const Vec2 &pos, double radius, const Color &color) noexcept :
    Entity(pos, radius, color) {
    type = CellType::VIRUS;

    flag = viruses;
    canEat = cfg::virus_canEat;
    avoidSpawningOn = cfg::virus_avoidSpawningOn;

    isSpiked = cfg::virus_isSpiked;
    isAgitated = cfg::virus_isAgitated;
}
void Virus::split(double angle, double radius) noexcept {
    // Set radius of splitting virus
    setRadius(radius);

    // Spawn new virus at splitting virus's position with same radius
    e_ptr newCell = map::spawnUnsafe<Virus>(_position, radius, _color);
    newCell->setVelocity(cfg::virus_initialAcceleration, angle);
    newCell->setCreator(newCell->nodeId());
}
void Virus::onDespawned() const noexcept {
    if (map::entities[type].size() < cfg::virus_startAmount)
        map::spawn<Virus>(randomPosition(), cfg::virus_baseRadius, cfg::virus_color);
}
void Virus::consume(e_ptr &prey) noexcept {
    if (map::entities[type].size() >= cfg::virus_maxAmount)
        return; // Max amount of viruses has been reached

    Entity::consume(prey);
    if (_radius >= cfg::virus_maxRadius)
        split(prey->velocity().angle(), cfg::virus_baseRadius);
}
Virus::~Virus() {
}