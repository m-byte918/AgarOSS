#include "Player.hpp"
#include "../Game/Map.hpp"
#include "../Player/Minion.hpp"
#include "../Player/PlayerBot.hpp"
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
void Player::setFullName(std::string name, bool isUCS2) noexcept {
    if (name.empty()) return;
    if (+name.back() == 0) name.pop_back();
    unsigned int maxLength = cfg::player_maxNameLength * (isUCS2 + 1);
    size_t skinStart = name.find(cfg::player_skinNameTags.front());
    size_t skinEnd = name.find(cfg::player_skinNameTags.back());
    if (skinStart != skinEnd && skinStart != std::string::npos && skinEnd != std::string::npos) {
        std::string skin = name.substr(skinStart + 1, skinEnd - 1);
        setSkinName(skin.substr(0, maxLength));
    }
    unsigned int length = std::min(maxLength, (unsigned int)name.size());
    setCellName(name.substr(_skinName.size() == 0 ? 0 : skinEnd + 1, length), isUCS2);
}
void Player::setSkinName(const std::string &name) noexcept {
    if (_skinName != name)
        _skinName = !name.size() ? "" : name;
}
void Player::setCellName(const std::string &name, bool isUCS2) noexcept {
    if (isUCS2) {
        _cellNameUCS2 = name;
        _cellNameUTF8 = "";
        for (unsigned char c : name) {
            if (+c != 0)
                _cellNameUTF8 += c;
        }
    } else {
        _cellNameUTF8 = name;
        _cellNameUCS2 = "";
        for (unsigned char c : name) {
            _cellNameUCS2 += c;
            _cellNameUCS2 += '\0';
        }
    }
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
const std::string &Player::cellNameUTF8() const noexcept {
    return _cellNameUTF8;
}
const std::string &Player::cellNameUCS2() const noexcept {
    return _cellNameUCS2;
}

//********************* UPDATING *********************//

void Player::update() {
    if (_state == PlayerState::PLAYING) {
        updateScore();
        updateCenter();
        updateViewBox();
    }
    else if (_state == PlayerState::FREEROAM) {
        // Update center
        Vec2 d = _mouse - _center;
        double dist  = d.length();
        double speed = std::min(dist, (double)cfg::player_maxFreeroamSpeed);
        if (speed > 1) {
            d = _center + (d / dist * speed); // Normalize and add speed
            _center.clampX(d.x, map::bounds().left(), map::bounds().right());
            _center.clampY(d.y, map::bounds().bottom(), map::bounds().top());
            updateViewBox();
            packetHandler.sendPacket(protocol->updateViewport({ viewBox.x(), viewBox.y() }, scale));
        }
    }
    else if (_state == PlayerState::SPECTATING) {
        // No players on map
        if (map::game->leaders.empty()) {
            setFreeroam();
            return;
        }
        // Set viewbox to largest player's viewbox
        Player *p = map::game->leaders.front();
        _center = p->_center;
        viewBox = p->viewBox;
        packetHandler.sendPacket(protocol->updateViewport({ viewBox.x(), viewBox.y() }, p->scale));
    }
    updateVisibleNodes();
    if (++lbUpdateTick > 25) {
        lbUpdateTick = 0;
        packetHandler.sendPacket(protocol->updateLeaderboardList());
    }
}
void Player::updateScore() {
    _score = 0;
    double total = 0;
    for (sptr<PlayerCell::Entity> cell : cells) {
        _score += cell->mass();
        total += cell->radius();
    }
    if (total > 0) 
        scale = std::pow((float)std::min(64.0 / total, 1.0), 0.4f);
}
void Player::updateCenter() {
    if (cells.empty()) return;

    Vec2 avg;
    for (sptr<PlayerCell::Entity> cell : cells)
        avg += cell->position();
    avg /= (double)cells.size();

    _center = (_center * 2 + avg) / 3;
}
void Player::updateViewBox() {
    Vec2 baseResolution(cfg::player_viewBoxWidth, cfg::player_viewBoxHeight);

    filteredScale = (9 * filteredScale + scale) / 10;
    Vec2 viewPort = baseResolution / filteredScale;
    
    viewBox.update(_center.x, _center.y, viewPort.x, viewPort.y);
}
void Player::updateVisibleNodes() {
    std::vector<e_ptr> delNodes, eatNodes, addNodes, updNodes;
    std::map<unsigned int, e_ptr> newVisibleNodes;

    for (Collidable *obj : map::quadTree.getObjectsInBound(viewBox)) {
        if (!obj->data.has_value()) continue;
        e_ptr entity = std::any_cast<e_ptr>(obj->data);
        if (visibleNodes.find(entity->nodeId()) == visibleNodes.end()) {
            addNodes.push_back(entity->shared);
        } else if (entity->state & needsUpdate) {
            if (entity.use_count() <= 6)
                entity->state &= ~needsUpdate;
            updNodes.push_back(entity->shared);
        }
        newVisibleNodes[entity->nodeId()] = entity->shared;
    }
    for (const auto &[nodeId, entity] : visibleNodes) {
        if (entity->state & isRemoved || 
            (newVisibleNodes.find(nodeId) == newVisibleNodes.end() && entity->creator() != id)) {
            if (entity->killerId())
                eatNodes.push_back(entity);
            delNodes.push_back(entity);
        }
    }
    visibleNodes = newVisibleNodes;

    // Send packet
    if (eatNodes.size() + updNodes.size() + delNodes.size() + addNodes.size() > 0)
        packetHandler.sendPacket(protocol->updateNodes(eatNodes, updNodes, delNodes, addNodes));
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
    for (sptr<PlayerCell::Entity> cell : cells) {
        // Too small to split
        if (cell->mass() <= cfg::playerCell_minMassToSplit)
            continue;
        cellsToSplit.push_back(cell->shared);
        if (cellsToSplit.size() + cells.size() >= cfg::player_maxCells)
            break;
    }
    for (e_ptr cell : cellsToSplit) {
        Vec2 diff = (_mouse - cell->position()).round();
        double angle = 0;

        if ((int)diff.squared() <= 1) {
            diff = { 1, 0 };
        } else {
            diff.normalize(); // not horizontal
            angle = diff.angle();
        }
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
    for (sptr<PlayerCell::Entity> cell : cells) {
        if (cell->radius() < cfg::playerCell_minRadiusToEject)
            continue;

        // Remove mass from creator
        cell->setMass(cell->mass() - ejectMass);

        Vec2 diff = _mouse - cell->position();
        double angle = 0;

        if (diff.squared() <= 1) {
            diff = { 1, 0 }; // horizontal shot
        } else {
            diff.normalize(); // not horizontal
            angle = diff.angle() + rand(-variation, variation);
        }
        // Formula for ejected mass efficiency:
        // mass = (e + ((f / 100) * e)) - e
        // == e * f / 100
        // size = sqrt(e * f / 100 * 100)
        // == sqrt(e * f)
        sptr<Ejected> ejected = map::spawn<Ejected>(
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
    visibleNodes.clear();
    // Should no longer be updated, remove from clients list
    if (owner == nullptr) {
        if (socket == nullptr) {
            server->playerBots.erase(std::find(server->playerBots.begin(),
                server->playerBots.end(), (PlayerBot*)this));
        } else {
            std::vector<Player*>::iterator index = std::find(server->clients.begin(),
                server->clients.end(), this);
            if (index != server->clients.end())
                server->clients.erase(index);
            if (protocol != nullptr)
                delete protocol;
        }
    } else {
        server->minions.erase(std::find(server->minions.begin(),
            server->minions.end(), (Minion*)this));
    }
    // Cache cell destination
    for (sptr<PlayerCell::Entity> cell : cells)
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
void Player::onSpawn() noexcept {
    spawn();
    packetHandler.sendPacket(protocol->clearAll());
    packetHandler.sendPacket(protocol->addNode(cells.back()->nodeId()));
    // Add starting minions (if any)
    if (cfg::server_minionsPerPlayer > 0 && minions.empty())
        map::game->commands.minion({ id, cfg::server_minionsPerPlayer });
}

void Player::spawn() noexcept {
    Vec2  position = randomPosition();
    Color color    = randomColor();
    float radius   = spawnRadius;

    // Chance to spawn from ejected mass
    if (cfg::player_chanceToSpawnFromEjected >= 1 && cfg::player_chanceToSpawnFromEjected <= 100) {
        std::vector<e_ptr> &ejectedCells = map::entities[Ejected::TYPE];
        if (rand(1, 100) <= cfg::player_chanceToSpawnFromEjected && !ejectedCells.empty()) {
            // Select random ejected cell
            e_ptr ejected = ejectedCells[rand(0, (int)ejectedCells.size() - 1)];
            if (ejected && !(ejected->state & isRemoved) && ejected->acceleration() < 1) {
                position = ejected->position();
                radius   = std::max(ejected->radius(), radius);
                color    = ejected->color();
                map::despawn(ejected);
            }
        }
    }
    // Spawn de cell
    sptr<PlayerCell> cell = map::spawn<PlayerCell>(position, radius, color);
    cells.push_back(cell);
    cell->setOwner(this);
    cell->setCreator(id);
    _state = PlayerState::PLAYING;
    _mouse = cell->position();
}
std::string Player::toString() noexcept {
    std::stringstream ss;

    ss << "owner: " << owner
        << "\nserver: " << server
        << "\nprotocol: " << protocol;
    if (protocol != nullptr) {
        ss << " {\n    buffer: {"
            << "\n        byteStr(): " << protocol->buffer.byteStr()
            << "\n        getReadOffset(): " << protocol->buffer.getReadOffset()
            << "\n        getWriteOffset(): " << protocol->buffer.getWriteOffset()
            << "\n    }"
            << "\n}";
    }
    ss << "\npacketHandler: {"
        << "\n    player: " << packetHandler.player
        << "\n}"
        << "\nsocket: " << socket;
    if (socket != nullptr) {
        ss << " {\n    canCork(): " << socket->canCork()
            << "\n    getAsyncSocketData(): " << socket->getAsyncSocketData()
            << "\n    getBufferedAmount(): " << socket->getBufferedAmount()
            << "\n    getLoopData(): " << socket->getLoopData()
            //<< "\n    getRemoteAddress(): " << socket->getRemoteAddress()
            << "\n    getUserData(): " << socket->getUserData()
            << "\n    isCorked(): " << socket->isCorked()
            << "\n}";
    }

    ss << "\n\nid: " << id
        << "\nprotocolNum: " << protocolNum
        << "\nisForceMerging: " << isForceMerging
        << "\ncontrollingMinions: " << controllingMinions
        << "\nspawnRadius: " << spawnRadius
        << "\ncells: {"
        << "\n    capacity(): " << cells.capacity()
        << "\n    max_size(): " << cells.max_size()
        << "\n    size(): " << cells.size()
        << "\n}"
        << "\nminions: {"
        << "\n    capacity(): " << minions.capacity()
        << "\n    max_size(): " << minions.max_size()
        << "\n    size(): " << minions.size()
        << "\n}"

        << "\n\nscore(): " << score()
        << "\nmouse(): " << mouse().toString()
        << "\ncenter(): " << center().toString()
        << "\nstate(): " << (unsigned int)state()
        << "\nskinName(): " << skinName()
        << "\ncellNameUTF8(): " << cellNameUTF8()
        << "\ncellNameUCS2(): " << cellNameUCS2()

        << "\n\nscale: " << scale
        << "\nfilteredScale: " << filteredScale
        << "\nlbUpdateTick: " << +lbUpdateTick

        << "\n\nvisibleNodes: {"
        << "\n    max_size(): " << visibleNodes.max_size()
        << "\n    size(): " << visibleNodes.size()
        << "\n}"

        << "\n\nviewBox: {"
        << "\n    x(): " << viewBox.x()
        << "\n    y(): " << viewBox.y()
        << "\n    width(): " << viewBox.width()
        << "\n    height(): " << viewBox.height()
        << "\n    halfWidth(): " << viewBox.halfWidth()
        << "\n    halfHeight(): " << viewBox.halfHeight()
        << "\n    left(): " << viewBox.left()
        << "\n    top(): " << viewBox.top()
        << "\n    right(): " << viewBox.right()
        << "\n    bottom(): " << viewBox.bottom()
        << "\n}";

    return ss.str();
}

Player::~Player() {
}