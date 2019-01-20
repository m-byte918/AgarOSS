#include "PlayerCell.hpp"
#include "../Game/Map.hpp"
#include "../Game/Game.hpp" // configs
#include "../Player/Player.hpp"

PlayerCell::PlayerCell(const Vec2 &pos, double radius, const Color &color) noexcept :
    Entity(pos, radius, color) {
    type = CellType::PLAYERCELL;

    flag = playercells;
    canEat = cfg::playerCell_canEat;
    avoidSpawningOn = cfg::playerCell_avoidSpawningOn;

    isSpiked = cfg::playerCell_isSpiked;
    isAgitated = cfg::playerCell_isAgitated;
}
void PlayerCell::move() noexcept {
    if (_owner->state() == PlayerState::DISCONNECTED)
        return;

    // Difference between centers
    Vec2 diff = (_owner->mouse() - _position).round();
    double distance = (int)diff.squared();

    // Not enough of a difference to move
    if (distance <= 1) return;

    // distance between mouse and cell
    distance = std::sqrt(distance);

    // https://imgur.com/a/H9s0J
    // s = min(d, 2.2 * (r^-0.4396754) * t * m) / d
    double speed = 2.2 * std::pow(_radius, -0.4396754); // speed per millisecond
    speed *= cfg::game_timeStep * cfg::playerCell_speedMultiplier; // speed per tick

    // limit the speed and check > 0 to prevent jittering
    speed = std::min(distance, speed) / distance;
    if (speed > 0)
        // Move to target at normalized speed then validate position
        setPosition(_position + diff * speed, true);
}
void PlayerCell::autoSplit() noexcept {
    if (_radius <= cfg::playerCell_maxRadius) return;

    double maxMass = toMass(cfg::playerCell_maxRadius);
    double remaining = cfg::player_maxCells - (int)_owner->cells.size();
    double splitTimes = std::min(std::ceil(_mass / maxMass), remaining);
    double splitRadius = toRadius(std::min(_mass / splitTimes, maxMass));

    if (_owner->cells.size() == cfg::player_maxCells - 1) {
        ++splitTimes;
        splitRadius *= INV_SQRT_2;
    }
    for (; splitTimes > 1; splitTimes--)
        split(rand(0.0, 2.0) * (double)MATH_PI, splitRadius);
    setRadius(splitRadius);
}
//                        sm
// (614  + 100) -> 345,  46,  33, 30, 29, 26, 22, 22, 22, 22, 20, 20, 20, 19, 19, 19
// (559  + 222) -> 392,  49,  24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24
// (712  + 100) -> 406,  51,  25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25
// (848  + 100) -> 479,  59,  30, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29
// (858  + 100) -> 475,  57,  29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29 
// (912  + 100) -> 512,  68,  32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32
// (954  + 100) -> 527,  65,  32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32
// (1004 + 100) -> 552,  68,  35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35
// (1183 + 100) -> 641,  39,  39, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29
// (1211 + 100) -> 655,  327, 40, 40, 22, 21, 21, 20, 20, 20, 20, 20, 20, 20, 20, 20
// (1222 + 100) -> 651,  327, 40, 40, 21, 21, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20
// (1225 + 222) -> 723,  361, 44, 44, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22
// (1544 + 100) -> 818,  408, 53, 50, 27, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25
// (1705 + 100) -> 900,  449, 59, 56, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28
// (2161 + 100) -> 1127, 540, 70, 70, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33
void PlayerCell::pop() noexcept {
    // rough draft
    int cellsLeft = cfg::player_maxCells - (int)_owner->cells.size();
    if (cellsLeft <= 0) return;

    std::vector<double> masses;
    std::vector<double> remainingMasses;
    masses.reserve(cellsLeft);
    remainingMasses.reserve(cellsLeft);

    double nextMass = _mass / 2;
    double minMass = toMass(cfg::playerCell_minRadiusToSplit);
    double splitMass = _mass / (cellsLeft + 1);
    double threshold = minMass * minMass;

    int amountOfBigSplits = (int)std::floor(_mass / threshold);
    if (amountOfBigSplits > 3)
        amountOfBigSplits = rand(4, 6);
    amountOfBigSplits = std::min(amountOfBigSplits, cellsLeft);

    auto distributeMass = [&]() {
        // Split into equal-sized pieces
        if (splitMass <= minMass) {
            remainingMasses = std::vector(cellsLeft, splitMass);
            // Equal-sized pieces are too small, decrease amount of splits
            if (splitMass <= cfg::playerCell_minVirusSplitMass) {
                cellsLeft = std::min(cellsLeft, (cellsLeft + 1) / 2);
                splitMass = _mass / cellsLeft;
                // Still too small -> repeat
                if (splitMass * 2 <= cfg::playerCell_minVirusSplitMass * 3) {
                    cellsLeft /= 2;
                    splitMass = _mass / cellsLeft;
                }
                remainingMasses = std::vector(cellsLeft - 1, splitMass);
            }
        // Too big to split into equal-sized pieces, add excess mass to one piece
        } else if (splitMass <= minMass * 2) {
            masses.push_back(splitMass);
            remainingMasses = std::vector(cellsLeft - (int)masses.size(), nextMass / (cellsLeft + 1));
        // Piece with excess mass is too big, divide it's mass in two.
        } else {
            masses.push_back(splitMass / 2);
            masses.push_back(masses.back());
            splitMass = nextMass / (cellsLeft + 1);
            if (splitMass >= minMass)
                splitMass = (nextMass - (masses.back() * 2)) / cfg::playerCell_minVirusSplitMass;
            splitMass = std::max(splitMass, cfg::playerCell_minVirusSplitMass);

            if (cellsLeft - (int)masses.size() < 0)
                remainingMasses = std::vector(cellsLeft, splitMass);
            else
                remainingMasses = std::vector(cellsLeft - (int)masses.size(), splitMass);
        }
    };
    if (amountOfBigSplits == 0) {
        distributeMass();
        if (masses.empty())
            nextMass = splitMass;
    } else if (amountOfBigSplits <= 3) {
        while (amountOfBigSplits > 0) {
            nextMass /= 2;
            masses.push_back(nextMass);
            amountOfBigSplits--;
        }
        distributeMass();
        nextMass = _mass / 2;
    }
    // Masses have been determined, skip larger splits
    if (amountOfBigSplits <= 3) {
        masses.insert(masses.end(), remainingMasses.begin(), remainingMasses.end());
        Logger::info("1: ",masses.size());
        setMass(nextMass);
    } else {
        // Larger splits
        double massLeft = nextMass;
        while (cellsLeft > 0) {
            if (amountOfBigSplits > 0) {
                if (nextMass / amountOfBigSplits <= cfg::playerCell_minVirusSplitMass * 2) {
                    amountOfBigSplits = 0;
                    continue;
                }
                nextMass /= 2;
                if (amountOfBigSplits == 1 && rand(0, 3) == 3) {
                    nextMass /= 2;
                    masses.push_back(nextMass);
                    massLeft -= nextMass;
                    cellsLeft--;
                }
                amountOfBigSplits--;
            } else {
                // Larger splits done -> fill remaining slots with small cells
                nextMass = std::max(massLeft / (cellsLeft + 1), cfg::playerCell_minVirusSplitMass);
            }
            masses.push_back(nextMass);
            massLeft -= nextMass;
            cellsLeft--;
        }
        setRadius(_radius * INV_SQRT_2);
        Logger::info("2: ", masses.size());
    }
    // Send masses flying at random angles
    for (double mass : masses)
        split(rand(0.0, 2.0) * (double)MATH_PI, toRadius(mass));
}
void PlayerCell::split(double angle, double radius) noexcept {
    // Spawn cell at splitting cell's position with new radius
    e_ptr newCell = map::spawnUnsafe<PlayerCell>(_position, radius, _color);
    newCell->setVelocity(cfg::playerCell_initialAcceleration, angle);
    newCell->setOwner(_owner);
    newCell->setCreator(_owner->id);

    // Add new cell to owner's cells
    _owner->cells.push_back(newCell);
    _owner->packetHandler.sendPacket(_owner->protocol->addNode(newCell->nodeId()));
}
void PlayerCell::consume(e_ptr &prey) noexcept {
    // Ejected cells ignore eat collision from the cell they were ejected
    // from for about 2 seconds (50 ticks) after initial boost
    if (prey->type == CellType::EJECTED && prey->creator() == _nodeId && prey->age() <= 50)
        return;
    Entity::consume(prey);
    // Split on a virus or mothercell
    if ((prey->type == CellType::VIRUS || prey->type == CellType::MOTHERCELL) && prey->isRemoved)
        pop();
}
void PlayerCell::update(unsigned long long tick) noexcept {
    move();

    // Update remerge
    double base = std::max(cfg::player_baseRemergeTime, std::floor(_radius * 0.2)) * 25;
    if (_owner->isForceMerging || cfg::player_baseRemergeTime <= 0)
        ignoreCollision = _acceleration < 150;
    else
        ignoreCollision = age() >= base;

    // Update decay once per second
    if ((tick % 25) != 0) return;

    double rate = cfg::playerCell_radiusDecayRate;
    if (rate <= 0) return;

    double newRadius = std::sqrt(radiusSquared() * rate);
    if (newRadius <= cfg::playerCell_baseRadius)
        return;

    setRadius(newRadius);
}
void PlayerCell::onDespawned() const noexcept {
    // Remove from owner's cells
    _owner->cells.erase(std::find(_owner->cells.begin(), _owner->cells.end(), shared));

    // Set owner as dead if no cells left
    if (_owner->cells.empty() && _owner->state() != PlayerState::DISCONNECTED)
        _owner->setDead();
}

PlayerCell::~PlayerCell() {
}