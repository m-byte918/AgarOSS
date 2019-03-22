#include "PlayerCell.hpp"
#include "../Game/Map.hpp"
#include "../Game/Game.hpp" // configs
#include "../Player/Player.hpp"

PlayerCell::PlayerCell(const Vec2 &pos, float radius, const Color &color) noexcept :
    Entity(pos, radius, color) {
    type = CellType::PLAYERCELL;

    flag = playercells;
    canEat = cfg::playerCell_canEat;
    avoidSpawningOn = cfg::playerCell_avoidSpawningOn;

    if (cfg::playerCell_isSpiked)   state |= isSpiked;
    if (cfg::playerCell_isAgitated) state |= isAgitated;
}
void PlayerCell::move() noexcept {
    if (speedMultiplier == 0) return;

    if (_owner && _owner->state() != PlayerState::DISCONNECTED)
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
    if (_mass <= cfg::playerCell_maxMass || cellAmountCache > cfg::player_maxCells) 
        return;

    unsigned int remaining  = cfg::player_maxCells - (int)cellAmountCache;
    unsigned int splitTimes = std::min((unsigned int)std::ceil(_mass / cfg::playerCell_maxMass), remaining);
    float splitRadius = toRadius(std::min(_mass / splitTimes, cfg::playerCell_maxMass));

    if (cellAmountCache == cfg::player_maxCells - 1) {
        ++splitTimes;
        splitRadius *= INV_SQRT_2;
    }
    for (; splitTimes > 1; splitTimes--)
        split(float(rand(0.0, 2.0) * MATH_PI), splitRadius);
    setRadius(splitRadius);
}
void PlayerCell::pop() noexcept {
    // rough draft
    int cellsLeft = cfg::player_maxCells - (int)cellAmountCache;
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
        while (cellAmountCache + masses.size() < cfg::player_maxCells) {
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
                nextMass;
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
    cellAmountCache = cfg::player_maxCells;
    for (const float &mass : masses)
        split(float(rand(0.0, 2.0) * MATH_PI), toRadius(mass));
}
void PlayerCell::split(double angle, float radius) noexcept {
    // Spawn cell at splitting cell's position with new radius
    e_ptr &newCell = map::spawnUnsafe<PlayerCell>(_position, radius, _color);
    newCell->setVelocity(cfg::playerCell_initialAcceleration, angle);
    newCell->setOwner(_owner);
    newCell->setCreator(_creatorId);
    newCell->cellAmountCache = cellAmountCache;
    newCell->speedMultiplier = speedMultiplier;

    if (_owner == nullptr || _owner->state() == PlayerState::DISCONNECTED) {
        newCell->speedMultiplier = 0;
        return;
    }
    // Add new cell to owner's cells
    _owner->cells.push_back(newCell.get());

    if (_owner->protocol != nullptr)
        _owner->packetHandler.sendPacket(_owner->protocol->addNode(newCell->nodeId()));
}
void PlayerCell::consume(e_ptr &prey) noexcept {
    // Ejected cells ignore eat collision from the cell they were ejected
    // from for about 2 seconds (50 ticks) after initial boost
    if (prey->type == CellType::EJECTED && prey->creator() == _nodeId && prey->age() <= 50)
        return;
    // Do not gain mass from bots
    if (!(prey->type == CellType::PLAYERCELL && 
        prey->mass() <= cfg::playerCell_minMassToSplit * 0.5f && 
        _mass >= cfg::playerCell_maxMass / cfg::playerCell_minMassToSplit))
        setMass(_mass + prey->mass());
    // Split on a virus or mothercell
    if (prey->type == CellType::VIRUS || prey->type == CellType::MOTHERCELL)
        pop();
    
    prey->setKiller(_nodeId); // prey was killed by this
    map::despawn(prey.get()); // remove prey from map
}
void PlayerCell::update(unsigned long long tick) noexcept {
    move();

    if (_owner != nullptr)
        cellAmountCache = _owner->cells.size();

    // Update remerge
    float base = std::max(cfg::player_baseRemergeTime, std::floor(_radius * 0.2f)) * 25;
    if ((_owner && _owner->isForceMerging) || cfg::player_baseRemergeTime <= 0)
        state = _acceleration < 150 ? (state | ignoreCollision) : (state & ~ignoreCollision);
    else
        state = age() >= base ? (state | ignoreCollision) : (state & ~ignoreCollision);

    // Update decay once per second
    if ((tick % 25) == 0) {
        if (cfg::playerCell_radiusDecayRate <= 0) return;

        float newRadius = std::sqrt(radiusSquared() * cfg::playerCell_radiusDecayRate);
        if (newRadius <= cfg::playerCell_baseRadius)
            return;

        setRadius(newRadius);
    }
}
void PlayerCell::onDespawned() noexcept {
    if (!_owner) return;

    // Remove from owner's cells
    _owner->cells.erase(std::find(_owner->cells.begin(), _owner->cells.end(), this));

    if (_owner->cells.empty()) {
        if (_owner->state() != PlayerState::DISCONNECTED) {
            _owner->setDead();
        } else {
            // NOW delete owner, their vector of 
            // cells no longer needs to be accessed
            delete _owner;
            _owner = nullptr;
        }
    }
}

PlayerCell::~PlayerCell() {
}