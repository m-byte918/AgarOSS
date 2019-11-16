#include "Virus.hpp"
#include "Ejected.hpp"
#include "MotherCell.hpp"
#include "PlayerCell.hpp"
#include "../Game/Map.hpp"
#include "../Game/Game.hpp" // configs
#include "../Player/Player.hpp"

PlayerCell::PlayerCell(const Vec2 &pos, float radius, const Color &color) noexcept :
    Entity(pos, radius, color) {

    flag = playercells;
    canEat = cfg::playerCell_canEat;
    avoidSpawningOn = cfg::playerCell_avoidSpawningOn;

    if (cfg::playerCell_isSpiked)   state |= isSpiked;
    if (cfg::playerCell_isAgitated) state |= isAgitated;
}
void PlayerCell::move() noexcept {
    if (speedMultiplier == 0) return;

    if (_owner->state() != PlayerState::DISCONNECTED)
        mouseCache = _owner->mouse();

    // Difference between centers
    Vec2 dir = (mouseCache - _position).round();
    double distance = (int)dir.squared();

    // Not enough of a difference to move
    if (distance <= 1) return;

    // distance between mouse and cell
    distance = std::sqrt(distance);

    // https://imgur.com/a/H9s0J
    // s = min(d, 2.2 * (r^-0.4396754) * t * m) / d
    double speed = 2.2 * std::pow(_radius, -0.4396754); // speed per millisecond
    speed *= cfg::game_timeStep * speedMultiplier; // speed per tick

    // limit the speed and check > 0 to prevent jittering
    speed = std::min(distance, speed) / distance;
    if (speed > 0)
        // Move to target at normalized speed then validate position
        setPosition(_position + dir * speed, true);
}
void PlayerCell::autoSplit() noexcept {
    if (_mass <= cfg::playerCell_maxMass || _owner->cells.size() > cfg::player_maxCells 
        || _owner->isForceMerging)
        return;

    unsigned int remaining  = cfg::player_maxCells - (int)_owner->cells.size();
    unsigned int splitTimes = std::min((unsigned int)std::ceil(_mass / cfg::playerCell_maxMass), remaining);
    float splitRadius = toRadius(std::min(_mass / splitTimes, cfg::playerCell_maxMass));

    if (_owner->cells.size() == cfg::player_maxCells - 1) {
        ++splitTimes;
        splitRadius *= INV_SQRT_2;
    }
    for (; splitTimes > 1; splitTimes--)
        split(float(rand(0.0, 2.0) * MATH_PI), splitRadius);
    setRadius(splitRadius);
}
void PlayerCell::pop() noexcept {
    // rough draft
    int cellsLeft = cfg::player_maxCells - (int)_owner->cells.size();
    if (cellsLeft <= 0) return;

    float splitMass = _mass / cellsLeft;
    std::vector<float> masses;
    masses.reserve(cellsLeft);

    if (splitMass <= cfg::playerCell_minMassToSplit) {
        unsigned int amount = 2;
        for (; _mass > cfg::playerCell_minMassToSplit * amount
            && amount < cfg::player_maxCells; amount *= 2);
        cellsLeft = std::min(cellsLeft, (int)amount);
        splitMass = _mass / cellsLeft;
        cellsLeft -= 1;
        setMass(splitMass);
    } else {
        float nextMass = _mass * 0.5f;
        float totalMass = nextMass;
        while (_owner->cells.size() + masses.size() < cfg::player_maxCells) {
            splitMass = nextMass / cfg::player_maxCells;
            if (splitMass < cfg::playerCell_minMassToSplit) {
                float totalProjectedMass = totalMass + splitMass * cellsLeft;
                float prevMass = nextMass;
                nextMass = _mass - totalProjectedMass;
                while (totalMass + (prevMass / 2) * cellsLeft > _mass && cellsLeft > 0) {
                    nextMass = prevMass / 2;
                    if (nextMass < cfg::playerCell_minMassToSplit)
                        break;
                    if (totalMass + nextMass + splitMass * (cellsLeft-1) > _mass)
                        nextMass -= totalMass + nextMass + splitMass * (cellsLeft-1) - _mass;
                    totalMass += nextMass;
                    masses.push_back(nextMass);
                    --cellsLeft;
                    prevMass = nextMass;
                }
                if (nextMass < cfg::playerCell_minMassToSplit) {
                    splitMass = std::max(nextMass, cfg::playerCell_minVirusSplitMass);
                    //Logger::warn(splitMass);
                    break;
                }
                if (totalMass + (prevMass / 2) * cellsLeft == _mass && cellsLeft > 0) {
                    prevMass /= cellsLeft;
                    if (prevMass < cfg::playerCell_minMassToSplit)
                        break;
                    for (; cellsLeft > 0; --cellsLeft)
                        masses.push_back(prevMass);
                }
                while (totalProjectedMass < _mass && cellsLeft > 0) {
                    if (nextMass < cfg::playerCell_minMassToSplit)
                        nextMass *= 2;
                    if (nextMass < cfg::playerCell_minMassToSplit)
                        break;
                    totalMass += nextMass;
                    masses.push_back(nextMass);
                    --cellsLeft;
                    totalProjectedMass = totalMass + splitMass * cellsLeft;
                }
                break;
            }
            if (cellsLeft == 1 && totalMass + (nextMass / 2) < _mass)
                nextMass = nextMass;
            else
                nextMass /= 2;
            totalMass += nextMass;
            masses.push_back(nextMass);
            --cellsLeft;
        }
        setRadius(_radius * INV_SQRT_2);
    }
    for (; splitMass < cfg::playerCell_minVirusSplitMass; splitMass *= 2);
    for (; cellsLeft > 0; --cellsLeft)
        masses.push_back(splitMass);
    // Send masses flying at random angles
    for (const float &mass : masses)
        split(float(rand(0.0, 2.0) * MATH_PI), toRadius(mass));
}
void PlayerCell::split(double angle, float radius) noexcept {
    // Spawn cell at splitting cell's position with new radius
    sptr<PlayerCell> newCell = map::spawn<PlayerCell>(_position + 20, radius, _color, false);
    newCell->setVelocity(cfg::playerCell_initialAcceleration, angle);
    newCell->setOwner(_owner);
    newCell->setCreator(_creatorId);
    newCell->speedMultiplier = _owner->state() == PlayerState::DISCONNECTED ? 0 : speedMultiplier;

    // Add new cell to owner's cells
    _owner->cells.push_back(newCell);

    if (_owner->socket != nullptr)
        _owner->packetHandler.sendPacket(_owner->protocol->addNode(newCell->nodeId()));
}
void PlayerCell::consume(e_ptr prey) noexcept {
    // Ejected cells ignore eat collision from the cell they were ejected
    // from for about 2 seconds (50 ticks) after initial boost
    if (prey->type == Ejected::TYPE && prey->creator() == _nodeId && prey->age() <= 50)
        return;
    // Do not gain mass from bots
    if (!(prey->type == PlayerCell::TYPE && 
        prey->mass() <= cfg::playerCell_minMassToSplit * 0.5f && 
        _mass >= cfg::playerCell_maxMass / cfg::playerCell_minMassToSplit))
        setMass(_mass + prey->mass());
    // Split on a virus or mothercell
    if (prey->type == Virus::TYPE || prey->type == MotherCell::TYPE)
        pop();
    prey->setKiller(_nodeId); // prey was killed by this
    map::despawn(prey); // remove prey from map
}
void PlayerCell::update() noexcept {
    move();
 
    // Update remerge
    float base = std::max(cfg::player_baseRemergeTime, std::floor(_radius * 0.2f)) * 25;
    if ((_owner && _owner->isForceMerging) || cfg::player_baseRemergeTime <= 0)
        state = _acceleration < 150 ? (state | ignoreCollision) : (state & ~ignoreCollision);
    else
        state = age() >= base ? (state | ignoreCollision) : (state & ~ignoreCollision);

    // Update decay once per second
    if (++decayTick > 25) {
        decayTick = 0;
        if (cfg::playerCell_radiusDecayRate <= 0) 
            return;
        float newRadius = std::sqrt(radiusSquared() * cfg::playerCell_radiusDecayRate);
        if (newRadius <= cfg::playerCell_baseRadius)
            return;
        setRadius(newRadius);
    }
}
void PlayerCell::onDespawned() noexcept {
    if (!_owner) return;

    // Remove from owner's cells
    _owner->cells.erase(std::find(_owner->cells.begin(), _owner->cells.end(), shared));

    if (_owner->cells.empty()) {
        if (_owner->state() != PlayerState::DISCONNECTED) {
            _owner->setDead();
        } else {
            // NOW it is safe to delete owner
            delete _owner;
            _owner = nullptr;
        }
    }
}

PlayerCell::~PlayerCell() {
}