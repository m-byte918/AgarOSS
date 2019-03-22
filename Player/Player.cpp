#include "Player.hpp"
#include "../Game/Map.hpp"
#include "../Player/Minion.hpp"
#include "../Modules/Logger.hpp"
#include "../Entities/Ejected.hpp"

//********************* SETTERS *********************//

Player::Player(Server *_server) : 
    server(_server) {
    id = prevPlayerId == 4294967295 ? 1 : ++prevPlayerId;
}
void Player::setDead() noexcept {
    _state = PlayerState::DEAD;
    _score = 0.0;
}
void Player::setFreeroam() noexcept {
    _state = PlayerState::FREEROAM;
    _score = 0.0;
    scale = cfg::player_maxFreeroamScale;
}
void Player::setSpectating() noexcept {
    _state = PlayerState::SPECTATING;
    _score = 0.0;
}
void Player::setSkinName(const std::string &name) noexcept {
    if (_skinName != name)
        _skinName = !name.size() ? "" : name;
}
void Player::setCellName(const std::string &name) noexcept {
    _cellName = name;
}

//********************* GETTERS *********************//

double Player::score() const noexcept {
    return _score;
}
const Vec2 &Player::mouse() const noexcept {
    return _mouse;
}
const Vec2 &Player::center() const noexcept {
    return _center;
}
const PlayerState &Player::state() const noexcept {
    return _state;
}
const std::string &Player::skinName() const noexcept {
    return _skinName;
}
const std::string &Player::cellName() const noexcept {
    return _cellName;
}

//********************* UPDATING *********************//

void Player::update(unsigned long long tick) {
    if (_state == PlayerState::DEAD) {
        updateVisibleNodes();
    }
    else if (_state == PlayerState::PLAYING) {
        updateScore();
        updateCenter();
        updateViewBox();
        updateVisibleNodes();
    }
    else if (_state == PlayerState::FREEROAM) {
        // Update _center
        Vec2 d = _mouse - _center;
        double dist  = d.length();
        double speed = std::min(dist, (double)cfg::player_maxFreeroamSpeed);

        if (speed > 1) {
            d = _center + (d / dist * speed); // normalize and add speed
            _center.x = std::min(std::max(map::bounds().left(), d.x), map::bounds().right());
            _center.y = std::min(std::max(map::bounds().bottom(), d.y), map::bounds().top());
            updateViewBox();
            packetHandler.sendPacket(protocol->updateViewport({ viewBox.x(), viewBox.y() }, scale));
        }
        updateVisibleNodes();
    }
    else if (_state == PlayerState::SPECTATING) {
        // Get largest player
        Player *largest = *std::max_element(server->clients.begin(), server->clients.end(), [](Player *a, Player *b) {
            return a->score() < b->score();
        });
        // No players on map
        if (largest->state() != PlayerState::PLAYING) {
            setFreeroam();
            return;
        }
        // Set viewbox to largest player's viewbox
        _center = largest->_center;
        viewBox = largest->viewBox;
        updateVisibleNodes();
        packetHandler.sendPacket(protocol->updateViewport({ viewBox.x(), viewBox.y() }, largest->scale));
    }
    else if (_state == PlayerState::DISCONNECTED) {
        updateDisconnection(tick);
    }
}
void Player::updateScore() {
    _score = 0;
    double total = 0;
    for (const Entity *cell : cells) {
        _score += cell->mass();
        total += cell->radius();
    }
    if (total > 0) 
        scale = std::pow((float)std::min(64 / total, 1.0), 0.4f);
}
void Player::updateCenter() {
    if (cells.empty()) return;

    Vec2 total;
    for (const Entity *cell : cells)
        total += cell->position();

    _center = total / (double)cells.size();
}
void Player::updateViewBox() {
    Vec2 baseResolution(cfg::player_viewBoxWidth, cfg::player_viewBoxHeight);

    filteredScale = (9 * filteredScale + scale) / 10;
    Vec2 viewPort = baseResolution / filteredScale;
    
    viewBox.update(_center.x, _center.y, viewPort.x, viewPort.y);
}
void Player::updateVisibleNodes() {
    std::vector<e_ptr> delNodes, eatNodes, addNodes, updNodes;
    std::map<unsigned long long, e_ptr> newVisibleNodes;

    for (Collidable *obj : map::quadTree.getObjectsInBound(viewBox)) {
        Entity *entity = std::any_cast<Entity*>(obj->data);
        if (entity->state & isRemoved) continue;
        if (visibleNodes.find(entity->nodeId()) == visibleNodes.end())
            addNodes.push_back(entity->shared);
        else if (entity->state & needsUpdate)
            updNodes.push_back(entity->shared);
        newVisibleNodes[entity->nodeId()] = entity->shared;
    }
    for (const auto &[nodeId, entity] : visibleNodes) {
        if (entity->state & isRemoved || newVisibleNodes.find(nodeId) == newVisibleNodes.end()) {
            if (entity->killerId() != 0)
                eatNodes.push_back(entity);
            delNodes.push_back(entity);
        }
    }
    visibleNodes = newVisibleNodes;

    // Send packet
    if (eatNodes.size() + updNodes.size() + delNodes.size() + addNodes.size() > 0)
        packetHandler.sendPacket(protocol->updateNodes(eatNodes, updNodes, delNodes, addNodes));
}
void Player::updateDisconnection(unsigned long long tick) {
    if (disconnectionTick == 0)
        disconnectionTick = tick;
    if (cfg::player_cellRemoveTime <= 0 || cells.empty()) {
        server->clients.erase(std::find(server->clients.begin(), server->clients.end(), this));
    } else if (tick - disconnectionTick >= cfg::player_cellRemoveTime * 25) {
        while (!cells.empty())
            map::despawn(cells.back());
    }
}

//********************* RECEIVED INFORMATION *********************//

void Player::onQKey() noexcept {
    controllingMinions = !controllingMinions;
    if (_state == PlayerState::SPECTATING)
        setFreeroam();
    else if (_state == PlayerState::FREEROAM)
        setSpectating();
}
void Player::onSplit() noexcept {
    if (controllingMinions) {
        for (Minion *m : minions)
            m->onSplit();
        return;
    }
    if (_state != PlayerState::PLAYING || cells.size() >= cfg::player_maxCells)
        return;
    
    std::vector<e_ptr> cellsToSplit;
    for (const Entity *cell : cells) {
        // Too small to split
        if (cell->mass() <= cfg::playerCell_minMassToSplit)
            continue;
        cellsToSplit.push_back(cell->shared);
        if (cellsToSplit.size() + cells.size() >= cfg::player_maxCells)
            break;
    }
    for (e_ptr cell : cellsToSplit) {
        Vec2 diff = (_mouse - cell->position()).round();

        if ((int)diff.squared() < 1)
            diff = { 1, 0 };
        else
            diff.normalize(); // not horizontal

        double angle = diff.angle();
        if (std::isnan(angle)) angle = MATH_PI * 0.5f;

        float halfRadius = cell->radius() * INV_SQRT_2;
        cell->setRadius(halfRadius); // Halve splitting cells radius
        cell->split(angle, halfRadius); // Split at half radius
    }
}
void Player::onEject() noexcept {
    if (controllingMinions) {
        for (Minion *m : minions)
            m->onEject();
        return;
    }
    if (_state != PlayerState::PLAYING)
        return;
    float ejectMass = toMass(cfg::ejected_baseRadius);
    float variation = cfg::playerCell_ejectAngleVariation;

    // Eject from all cells if possible
    for (Entity *cell : cells) {
        if (cell->radius() < cfg::playerCell_minRadiusToEject)
            continue;

        // Remove mass from creator
        cell->setMass(cell->mass() - ejectMass);

        Vec2 diff = _mouse - cell->position();
        if (diff.squared() < 1)
            diff = { 1, 0 }; // horizontal shot
        else
            diff.normalize(); // not horizontal

        double angle = diff.angle() + rand(-variation, variation);
        if (std::isnan(angle)) angle = MATH_PI * 0.5f;

        // Formula for ejected mass efficiency:
        // mass = (e + ((f / 100) * e)) - e
        // == e * f / 100
        // size = sqrt(e * f / 100 * 100)
        // == sqrt(e * f)
        e_ptr ejected = map::spawn<Ejected>(
            cell->position() + diff * cell->radius(), 
            std::sqrt(ejectMass * cfg::ejected_efficiency),
            cell->color()
        );
        ejected->setVelocity(cfg::ejected_initialAcceleration, angle);
        ejected->setCreator(cell->nodeId());
    }
}
void Player::onSpectate() noexcept {
    if (_state != PlayerState::PLAYING)
        setSpectating();
}
void Player::onDisconnection() noexcept {
    _state = PlayerState::DISCONNECTED;

    // Cache cell destination
    for (Entity *cell : cells)
        cell->mouseCache = cell->position();
    // Remove minions
    while (!minions.empty())
        minions.back()->onDisconnection();
}
void Player::onTarget(const Vec2 &target) noexcept {
    if (_state == PlayerState::DEAD) return;
    if (_mouse != target) _mouse = target;

    updateCenter();
}
void Player::onSpawn(std::string name) noexcept {
    if (_state == PlayerState::PLAYING) return;

    spawn(name);

    // Send update to client
    visibleNodes.clear();
    packetHandler.sendPacket(protocol->clearAll());
    packetHandler.sendPacket(protocol->addNode(cells.back()->nodeId()));
}

void Player::spawn(std::string name) noexcept {
    if (!name.empty()) {
        if (+name.back() == 0) name.pop_back();
        size_t skinStart = name.find('<');
        size_t skinEnd = name.find('>');
        if (skinStart != skinEnd && skinStart != -1 && skinEnd != -1) {
            std::string skin = name.substr(skinStart + 1, skinEnd - 1);
            setSkinName(skin.substr(0, cfg::player_maxNameLength));
        }
        setCellName(name.substr(_skinName.size() == 0 ? 0 : skinEnd + 1, cfg::player_maxNameLength));
    }
    Vec2  position = randomPosition();
    Color color    = randomColor();
    float radius   = spawnRadius;

    // Chance to spawn from ejected mass
    if (cfg::player_chanceToSpawnFromEjected >= 1 && cfg::player_chanceToSpawnFromEjected <= 100) {
        std::vector<e_ptr> &ejectedCells = map::entities[CellType::EJECTED];
        if (rand(1, 100) <= cfg::player_chanceToSpawnFromEjected && !ejectedCells.empty()) {
            // Select random ejected cell
            Entity *ejected = ejectedCells[rand(0, (int)ejectedCells.size() - 1)].get();

            if (ejected && !(ejected->state & isRemoved) && ejected->acceleration() < 1) {
                position = ejected->position();
                radius   = std::max(ejected->radius(), radius);
                color    = ejected->color();
                map::despawn(ejected);
            }
        }
    }
    // Spawn de cell
    e_ptr cell = map::spawn<PlayerCell>(position, radius, color);
    cells.push_back(cell.get());
    cell->setOwner(this);
    cell->setCreator(id);
    _state = PlayerState::PLAYING;
    _mouse = cell->position();
}

Player::~Player() {
}