#include "PlayerCell.hpp"
#include "../Player.hpp"
#include "../Game/Map.hpp"
#include "../Game/Game.hpp" // configs
#include "../Packets/AddNode.hpp" // split()

PlayerCell::PlayerCell(const Vector2 &pos, double radius, const Color &color) noexcept :
    Entity(pos, radius, color) {
    type = CellType::PLAYERCELL;

    flag = playercells;
    canEat = cfg::playerCell_canEat;
    avoidSpawningOn = cfg::playerCell_avoidSpawningOn;

    isSpiked = cfg::playerCell_isSpiked;
    isAgitated = cfg::playerCell_isAgitated;
}
void PlayerCell::move() noexcept {
    // Difference between centers
    Vector2 diff = _owner->getMouse() - _position;
    double distance = diff.squared();

    // Not enough of a difference to move
    if ((int)distance < 1) return;

    // distance between mouse and cell
    distance = std::sqrt(distance);

    // https://imgur.com/a/H9s0J
    // s = min(d, 2.2 * (r^-0.4396754) * t * m) / d
    double speed = 2.2 * std::pow(_radius, -0.4396754); // speed per millisecond
    speed *= cfg::game_timeStep * cfg::playerCell_speedMultiplier; // speed per tick

    // limit the speed and check > 0 to prevent jittering
    if (speed = std::min(distance, speed) / distance)
        // Move to target at normalized speed then validate position
        setPosition(_position + diff * speed, true);
}
void PlayerCell::split(double angle) noexcept {
    // Halve splitting cells radius
    double halfRadius = _radius * INV_SQRT_2;
    setRadius(halfRadius);

    // Spawn cell at splitting cell's position with half its radius
    e_ptr newCell = map::spawn<PlayerCell>(_position + 40, halfRadius, _color);
    newCell->setVelocity(cfg::playerCell_initialAcceleration, angle);
    newCell->setOwner(_owner);
    newCell->setCreator(_nodeId);

    // Add new cell to owner's cells
    _owner->cells.push_back(newCell);
    _owner->packetHandler.sendPacket(AddNode(newCell->nodeId()));
}
void PlayerCell::update(unsigned long long tick) noexcept {
    move();

    // Update decay once per second
    if ((tick % 25) != 0) return;

    double rate = cfg::playerCell_radiusDecayRate;
    if (rate <= 0) return;

    double newRadius = std::sqrt(radiusSquared() * rate);
    if (newRadius <= cfg::playerCell_baseRadius)
        return;

    setRadius(newRadius);
}

PlayerCell::~PlayerCell() {
}