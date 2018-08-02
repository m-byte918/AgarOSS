#include "Map.hpp"
#include "../Player.hpp"
#include "../Modules/Logger.hpp"

QuadTree Map::quadTree;

std::vector<std::vector<Entity*>> Map::entities{
    std::vector<Food::Entity*>(),
    std::vector<Virus::Entity*>(),
    std::vector<Ejected::Entity*>(),
    std::vector<MotherCell::Entity*>(),
    std::vector<PlayerCell::Entity*>()
};

void Map::init() {
    Logger::info("Initializing Map...");

    quadTree = QuadTree({
        0,
        0,
        config["game"]["mapWidth"],
        config["game"]["mapHeight"]
    }, config["game"]["quadTreeLeafCapacity"], config["game"]["quadTreeMaxDepth"]);
    
    // Spawn starting entities
    int foodStartAmount = config["food"]["startAmount"];
    int virusStartAmount = config["virus"]["startAmount"];

    while (entities[CellType::FOOD].size() < foodStartAmount)
        spawnFood();
    while (entities[CellType::VIRUS].size() < virusStartAmount)
        spawnVirus();
}

const Rect &Map::getBounds() noexcept {
    return quadTree.getBounds();
}

Food *Map::spawnFood(const Vector2 &pos, double size, const Color &color) noexcept {
    return spawnEntity<Food>(pos, size, color);
}

Virus *Map::spawnVirus(const Vector2 &pos, double size, const Color &color) noexcept {
    return spawnEntity<Virus>(pos, size, color);
}
Virus *Map::spawnVirus(double size, const Color &color) noexcept {
    return spawnEntity<Virus>(getSafePosition(size), size, color);
}

Ejected *Map::spawnEjected(const Vector2 &pos, double size, const Color &color) noexcept {
    return spawnEntity<Ejected>(pos, size, color);
}

MotherCell *Map::spawnMotherCell(const Vector2 &pos, double size, const Color &color) noexcept {
    return spawnEntity<MotherCell>(pos, size, color);
}
MotherCell *Map::spawnMotherCell(double size, const Color &color) noexcept {
    return spawnEntity<MotherCell>(getSafePosition(size), size, color);
}

PlayerCell *Map::spawnPlayerCell(const Vector2 &pos, double size, const Color &color) noexcept {
    return spawnEntity<PlayerCell>(pos, size, color);
}
PlayerCell *Map::spawnPlayerCell(double size, const Color &color) noexcept {
    return spawnEntity<PlayerCell>(getSafePosition(size), size, color);
}

void Map::removeEntity(Entity *entity) {
    std::vector<Entity*> *vec = &entities[entity->cellType];
    auto index = std::find(vec->begin(), vec->end(), entity);

    if (index == vec->end()) {
        Logger::error("Cannot remove entity that does not exist");
        return;
    }
    entity->isRemoved = true;
    quadTree.remove(&entity->obj);

    vec->erase(index);
    entity->onRemove();
    /*delete entity;
    entity = nullptr;*/
}

void Map::updateObject(Entity *entity) {
    Rect *bounds = &entity->obj.bound;
    double size = entity->getSize() * 2;
    Vector2 newPos = entity->getPosition();

    // Check if size and position are the same as before
    if (newPos == Vector2(bounds->x(), bounds->y()) && bounds->width() == size) {
        entity->needsUpdate = false;
        return;
    }
    bounds->update(newPos.x, newPos.y, size, size);
    quadTree.update(&entity->obj);
    entity->needsUpdate = true;
}

void Map::update(unsigned long long tick) {
    // Update food
    for (Food::Entity *food : entities[CellType::FOOD]) {
        updateObject(food);
    }
    // Update viruses
    for (Virus::Entity *virus : entities[CellType::VIRUS]) {
        updateObject(virus);
    }
    // Update playercells
    for (PlayerCell::Entity *playerCell : entities[CellType::PLAYERCELL]) {
        updateObject(playerCell);

        // Update decay once per second
        if (((tick + 3) % 25) == 0)
            playerCell->updateDecay();

        // Update collisions (experimental)
        for (Collidable *obj : quadTree.getObjectsInBound(playerCell->obj.bound)) {
            Entity *cell = std::any_cast<Entity*>(obj->data);
            if (!playerCell->intersects(cell)) continue;
            playerCell->consume(cell);
            removeEntity(cell);
        }
    }
}

// some notes regarding vanilla servers:
// viruses seemingly have a set amount of attempts to spawn safely (maybe make this the total amount of playercells?)
// viruses, mothercells, and playercells are checked for collision with other playercells before spawned, nothing else
// experimental mode: playercells have 0% chance to spawn inside mothercell
Vector2 Map::getSafePosition(double size) noexcept {
    Vector2 pos = getRandomPosition();
    //size *= 2; // width/height of the circle
    std::vector<Collidable*> found = quadTree.getObjectsInBound({ pos.x, pos.y, size * 2, size * 2 });

    unsigned attempts = entities[CellType::PLAYERCELL].size();

    for (unsigned i = 0; i < attempts && std::any_of(found.begin(), found.end(), [&](Collidable *obj) {
        return std::any_cast<PlayerCell::Entity*>(obj->data)->intersects(pos, size);
    }); ++i) {
        pos = getRandomPosition();
        found = quadTree.getObjectsInBound({ pos.x, pos.y, size * 2, size * 2 });
    }
    return pos;
}

template <typename T>
T *Map::spawnEntity(const Vector2 &pos, double size, const Color &color) noexcept {
    T *entity = new T(pos, size, color);
    entity->obj = Collidable({ pos.x, pos.y, size * 2, size * 2 }, (Entity*)entity);

    quadTree.insert(&entity->obj);
    entities[entity->cellType].push_back(entity);
    return entity;
}

void Map::clear() {
    Logger::warn("Clearing Map...");

    quadTree.clear();
    for (std::vector<Entity*> entityVec : entities) {
        for (Entity *entity : entityVec) {
            delete entity;
            entity = nullptr;
        }
        entityVec.clear();
    }
}