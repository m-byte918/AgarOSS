#include "Player.hpp"
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Modules/Logger.hpp"
#include "Packets/Packet.hpp"
#include "Packets/AddNode.hpp"
#include "Packets/ClearAll.hpp"
#include "Packets/UpdateNodes.hpp"
#include "Entities/Ejected.hpp"

Player::Player(uWS::WebSocket<uWS::SERVER> *_socket) :
    socket(_socket),
    packetHandler(PacketHandler(this)) {
}

const std::string &Player::getSkinName() const noexcept {
    return skinName;
}
const std::string &Player::getCellName() const noexcept {
    return cellName;
}
void Player::setSkinName(const std::string &name) noexcept {
    if (skinName != name)
        skinName = !name.size() ? "" : name;
}
void Player::setCellName(const std::string &name) noexcept {
    if (cellName == name)
        return;
    if (!name.size() || (name.size() == 1 && name[0] == 0))
        cellName = "An unnamed cell";
    else
        cellName = name;
}

double Player::getScore() const noexcept {
    return score;
}
Vector2 Player::getMouse() const noexcept {
    return mouse;
}
const PlayerState &Player::getState() const noexcept {
    return state;
}
const Vector2 &Player::getCenter() const noexcept {
    return center;
}

void Player::update() {
    if (state == PlayerState::PLAYING) {
        updateScale();
        updateCenter();
        updateViewBox();
        updateVisibleNodes();
    }
}
void Player::updateScale() {
    score = 0; // reset temporarily
    for (const e_ptr &cell : cells)
        score += cell->getRadius();

    if (score == 0) {
        scale = cfg::player_minViewBoxScale;
    } else {
        scale = std::pow(std::min(64 / score, 1.0), 0.4);
        score = toMass(score);
    }
}
void Player::updateCenter() {
    if (cells.empty()) return;

    Vector2 total;
    for (const e_ptr &cell : cells)
        total += cell->getPosition();

    center = total / cells.size();
}
void Player::updateViewBox() {
    Vector2 baseResolution(cfg::player_viewBoxWidth, cfg::player_viewBoxHeight);

    filteredScale = (9 * filteredScale + scale) / 10;
    Vector2 viewPort = baseResolution / filteredScale;
    
    viewBox.update(center.x, center.y, viewPort.x, viewPort.y);
}

void Player::updateVisibleNodes() {
    std::vector<e_ptr> delNodes, eatNodes, addNodes, updNodes;
    std::map<unsigned long long, e_ptr> newVisibleNodes;

    // This can be optimized further
    for (Collidable *obj : map::quadTree.getObjectsInBound(viewBox)) {
        const e_ptr &entity = std::any_cast<e_ptr>(obj->data);
        if (visibleNodes.find(entity->nodeId()) == visibleNodes.end())
            addNodes.push_back(entity);
        else if (entity->needsUpdate)
            updNodes.push_back(entity);
        newVisibleNodes[entity->nodeId()] = entity;
    }
    for (const auto &[nodeId, entity] : visibleNodes) {
        if (newVisibleNodes.find(nodeId) != newVisibleNodes.end() || !entity->isRemoved)
            continue;
        if (entity->killerId() != 0)
            eatNodes.push_back(entity);
        delNodes.push_back(entity);
    }
    visibleNodes = newVisibleNodes;

    // Send packet
    if (eatNodes.size() + updNodes.size() + delNodes.size() + addNodes.size() > 0)
        packetHandler.sendPacket(UpdateNodes(this, eatNodes, updNodes, delNodes, addNodes));
}

void Player::onSpawn(std::string name) noexcept {
    if (state == PlayerState::PLAYING) return;

    // Remove whitespaces
    /*for (unsigned i = 0; i < name.size(); ++i)
        name.erase(name.begin() + i + 1);*/
    /*for (unsigned i = 0; i < name.size(); ++i)
        Logger::debug(+*(name.begin() + i));*/
    
    name.pop_back();

    // Get skin name
    std::string skin;
    size_t skinStart = name.find('<');
    size_t skinEnd = name.find('>');
    if (skinStart != skinEnd && skinStart != -1 && skinEnd != -1)
        skin = name.substr(skinStart + 1, skinEnd - 1);

    setSkinName(skin.substr(0, cfg::player_maxNameLength));
    setCellName(name.substr(skin.size() == 0 ? 0 : skinEnd + 1, cfg::player_maxNameLength));

    packetHandler.sendPacket(ClearAll());

    e_ptr cell = map::spawn<PlayerCell>(
        randomPosition(), cfg::playerCell_baseRadius, randomColor()
    );
    cells.push_back(cell);
    cell->setOwner(this);

    packetHandler.sendPacket(AddNode(cell->nodeId()));

    state = PlayerState::PLAYING;
    mouse = cell->getPosition();
    update(); // Initial update
}

void Player::onSpectate() noexcept {
    if (state != PlayerState::PLAYING)
        state = PlayerState::SPECTATING;
}
void Player::onTarget(const Vector2 &target) noexcept {
    if (state == PlayerState::DEAD) return;
    if (mouse != target) mouse = target;

    updateCenter();
}
void Player::onSplit() noexcept {
    if (state != PlayerState::PLAYING)
        return;
    
    std::vector<e_ptr> cellsToSplit;
    for (const e_ptr &cell : cells) {
        // Too small to split
        if (cell->getRadius() <= cfg::playerCell_minRadiusToSplit)
            continue;
        cellsToSplit.push_back(cell);
        if (cellsToSplit.size() + cells.size() >= cfg::player_maxCells)
            break;
    }
    for (e_ptr cell : cellsToSplit) {
        Logger::debug("called");
        Vector2 d = mouse - cell->getPosition();

        if (d.squared() < 1)
            d = { 1, 0 };

        cell->split(d.angle());
    }
}
void Player::onQKey() noexcept {
    if (state == PlayerState::SPECTATING)
        state = PlayerState::FREEROAM;
    else if (state == PlayerState::FREEROAM)
        state = PlayerState::SPECTATING;
}

// Note regarding vanilla servers: Ejected mass does not collide with 
// its ejector until it has completely stopped acelerating
void Player::onEject() noexcept {
    if (state != PlayerState::PLAYING)
        return;
    double ejectMass = toMass(cfg::ejected_baseRadius);
    double variation = cfg::playerCell_ejectAngleVariation;

    // Eject from all cells if possible
    for (e_ptr cell : cells) {
        if (cell->getRadius() < cfg::playerCell_minRadiusToEject)
            continue;

        // Remove mass from creator
        cell->setMass(cell->getMass() - ejectMass);

        Vector2 diff = mouse - cell->getPosition();
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
            cell->getPosition() + diff * cell->getRadius(), 
            std::sqrt(ejectMass * cfg::ejected_efficiency),
            cell->getColor()
        );
        ejected->setVelocity(cfg::ejected_initialAcceleration, angle);
        ejected->setCreator(cell->nodeId());
    }
}

void Player::onDisconnection() noexcept {
    state = PlayerState::DISCONNECTED;
    for (e_ptr cell : cells) {
        map::despawn(cell);
    }
    cells.clear();
}

Player::~Player() {
    onDisconnection();
}
