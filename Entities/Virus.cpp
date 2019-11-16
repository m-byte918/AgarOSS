#include "Virus.hpp"
#include "../Game/Map.hpp"
#include "../Game/Game.hpp" // configs

Virus::Virus(const Vec2 &pos, float radius, const Color &color) noexcept :
    Entity(pos, radius, color) {

    flag = viruses;
    canEat = cfg::virus_canEat;
    avoidSpawningOn = cfg::virus_avoidSpawningOn;

    if (cfg::virus_isSpiked)   state |= isSpiked;
    if (cfg::virus_isAgitated) state |= isAgitated;
}
void Virus::split(double angle, float radius) noexcept {
    // Set radius of splitting virus
    setRadius(radius);

    // Spawn new virus at splitting virus's position with same radius
    sptr<Virus> newCell = map::spawn<Virus>(_position, radius, _color, false);
    newCell->setVelocity(cfg::virus_initialAcceleration, angle);
    newCell->setCreator(newCell->nodeId());
}
void Virus::onDespawned() noexcept {
    if (map::entities[type].size() < cfg::virus_startAmount)
        map::spawn<Virus>(randomPosition(), cfg::virus_baseRadius, cfg::virus_color);
}
void Virus::consume(e_ptr prey) noexcept {
    if (map::entities[type].size() >= cfg::virus_maxAmount)
        return; // Max amount of viruses has been reached

    Entity::consume(prey);
    if (_radius >= cfg::virus_maxRadius)
        split(_acceleration < 1 ? prey->velocity().angle() : 0, cfg::virus_baseRadius);
}
Virus::~Virus() {
}