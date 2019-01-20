#include "Player.hpp"
#include "../Game/Map.hpp"
#include "../Game/Game.hpp"
#include "../Modules/Logger.hpp"
#include "../Entities/Ejected.hpp"

//********************* SETTERS *********************//

Player::Player(uWS::WebSocket<uWS::SERVER> *_socket) :
    socket(_socket),
    packetHandler(PacketHandler(this)) {
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
    if (_cellName == name)
        return;
    if (!name.size() || (name.size() == 1 && name[0] == 0))
        _cellName = "An unnamed cell";
    else
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
        updateScale();
        updateCenter();
        updateViewBox();
        updateVisibleNodes();
    }
    else if (_state == PlayerState::FREEROAM) {
        // Update _center
        Vec2 d = _mouse - _center;
        double dist  = d.length();
        double speed = std::min(dist, cfg::player_maxFreeroamSpeed);

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
        if (disconnectionTick == 0)
            disconnectionTick = tick;
        if (cfg::player_cellRemoveTime <= 0 || cells.empty()) {
            server->clients.erase(std::find(server->clients.begin(), server->clients.end(), this));
        } else if (tick - disconnectionTick >= cfg::player_cellRemoveTime * 25) {
            while (!cells.empty())
                map::despawn(cells.back());
        }
    }
}
void Player::updateScale() {
    _score = 0;
    double total = 0;
    for (const e_ptr &cell : cells) {
        _score += cell->mass();
        total += cell->radius();
    }
    if (total > 0) 
        scale = std::pow((float)std::min(64 / total, 1.0), 0.4f);
}
void Player::updateCenter() {
    if (cells.empty()) return;

    Vec2 total;
    for (const e_ptr &cell : cells)
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

    // This can be optimized further
    for (Collidable *obj : map::quadTree.getObjectsInBound(viewBox)) {
        Entity *entity = std::any_cast<Entity*>(obj->data);
        if (visibleNodes.find(entity->nodeId()) == visibleNodes.end())
            addNodes.push_back(entity->shared);
        if (entity->needsUpdate)
            updNodes.push_back(entity->shared);
        newVisibleNodes[entity->nodeId()] = entity->shared;
    }
    for (const auto &[nodeId, entity] : visibleNodes) {
        if (!entity->isRemoved) continue;
        if (entity->killerId() != 0)
            eatNodes.push_back(entity);
        delNodes.push_back(entity);
    }
    visibleNodes = newVisibleNodes;

    // Send packet
    if (eatNodes.size() + updNodes.size() + delNodes.size() + addNodes.size() > 0)
        packetHandler.sendPacket(protocol->updateNodes(eatNodes, updNodes, delNodes, addNodes));
}

//********************* RECEIVED INFORMATION *********************//

void Player::onQKey() noexcept {
    if (_state == PlayerState::SPECTATING)
        setFreeroam();
    else if (_state == PlayerState::FREEROAM)
        setSpectating();
}
void Player::onSplit() noexcept {
    if (_state != PlayerState::PLAYING)
        return;
    if (cells.size() >= cfg::player_maxCells)
        return;
    
    std::vector<e_ptr> cellsToSplit;
    for (const e_ptr &cell : cells) {
        // Too small to split
        if (cell->radius() <= cfg::playerCell_minRadiusToSplit)
            continue;
        cellsToSplit.push_back(cell);
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

        double halfRadius = cell->radius() * INV_SQRT_2;
        cell->setRadius(halfRadius); // Halve splitting cells radius
        cell->split(angle, halfRadius); // Split at half radius
    }
}
void Player::onEject() noexcept {
    // Note regarding vanilla servers: Ejected mass does not collide 
    // with its ejector until it has completely stopped acelerating
    if (_state != PlayerState::PLAYING)
        return;
    double ejectMass = toMass(cfg::ejected_baseRadius);
    double variation = cfg::playerCell_ejectAngleVariation;

    // Eject from all cells if possible
    for (e_ptr &cell : cells) {
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
    delete protocol;
}
void Player::onTarget(const Vec2 &target) noexcept {
    if (_state == PlayerState::DEAD) return;
    if (_mouse != target) _mouse = target;

    updateCenter();
}
void Player::onSpawn(std::string name) noexcept {
    if (_state == PlayerState::PLAYING) return;

    // Get skin name
    name.pop_back();
    std::string skin;
    size_t skinStart = name.find('<');
    size_t skinEnd = name.find('>');
    if (skinStart != skinEnd && skinStart != -1 && skinEnd != -1)
        skin = name.substr(skinStart + 1, skinEnd - 1);

    setSkinName(skin.substr(0, cfg::player_maxNameLength));
    setCellName(name.substr(skin.size() == 0 ? 0 : skinEnd + 1, cfg::player_maxNameLength));

    Vec2   position = randomPosition();
    Color  color    = randomColor();
    double radius   = cfg::playerCell_baseRadius;

    // Chance to spawn from ejected mass
    if (cfg::player_chanceToSpawnFromEjected >= 1 && cfg::player_chanceToSpawnFromEjected <= 100) {
        std::vector<e_ptr> &ejectedCells = map::entities[CellType::EJECTED];
        if (rand(1, 100) <= cfg::player_chanceToSpawnFromEjected && !ejectedCells.empty()) {
            // Select random ejected cell
            e_ptr ejected = ejectedCells[rand(0, (int)ejectedCells.size())];

            if (!ejected->isRemoved && ejected->acceleration() < 1) {
                position = ejected->position();
                radius   = std::max(ejected->radius(), radius);
                color    = ejected->color();
                map::despawn(ejected);
            }
        }
    }
    // Spawn de cell
    e_ptr cell = map::spawn<PlayerCell>(position, radius, color);
    cells.push_back(cell);
    cell->setOwner(this);
    cell->setCreator(id);
    visibleNodes.clear();
    packetHandler.sendPacket(protocol->clearAll());
    packetHandler.sendPacket(protocol->addNode(cell->nodeId()));

    // Initial update
    _state = PlayerState::PLAYING;
    _mouse = cell->position();
    update(0);
}

Player::~Player() {
    onDisconnection();
}