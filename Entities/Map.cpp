#include "Map.hpp"
#include "../Player.hpp"
#include "../Modules/Logger.hpp"

namespace map {

std::vector<std::vector<Entity*>> entities {
    std::vector<Food::Entity*>(),
    std::vector<Virus::Entity*>(),
    std::vector<Ejected::Entity*>(),
    std::vector<MotherCell::Entity*>(),
    std::vector<PlayerCell::Entity*>()
};

QuadTree quadTree;

void init() {
    Logger::info("Initializing Map...");

    quadTree = QuadTree({
        0,
        0,
        config["game"]["mapWidth"],
        config["game"]["mapHeight"]
    }, config["game"]["quadTreeLeafCapacity"], config["game"]["quadTreeMaxDepth"]);

    // Spawn starting food
    int amount = config["food"]["startAmount"];
    double radius = config["food"]["baseRadius"];
    while (entities[CellType::FOOD].size() < amount)
        spawn<Food>(randomPosition(), radius, randomColor());

    // Spawn starting viruses
    amount = config["virus"]["startAmount"];
    radius = config["virus"]["baseRadius"];
    const Color virusColor = config["virus"]["color"];
    while (entities[CellType::VIRUS].size() < amount)
        spawn<Virus>(randomPosition(), radius, virusColor);
}

const Rect &getBounds() noexcept {
    return quadTree.getBounds();
}

template <typename T>
T *spawn(Vector2 &pos, double radius, const Color &color) noexcept {
    T *entity = new T(pos, radius, color); // Initial
    double wh = radius * 2; // Width/height of circular shape
    std::vector<Collidable*> found; // Entities within initial bounds

    if (entity->avoidSpawningOn == nothing)
        goto done; // Can spawn near any type, return as-is
    
    // Get entities near this one
    found = quadTree.getObjectsInBound({ pos.x, pos.y, wh, wh });
    if (found.empty()) goto done; // Safe

    // (experimental) attempts=size of the vector the entity belongs to
    const unsigned attempts = entities[entity->type].size();

    // Get safe position
    for (unsigned i = 0; i < attempts && std::any_of(found.begin(), found.end(), [&](Collidable *obj) {
        Entity *cell = std::any_cast<Entity*>(obj->data);
        return (entity->avoidSpawningOn & cell->flag) && cell->intersects(pos, radius);
    }); ++i) {
        // Retry
        pos = randomPosition();
        found = quadTree.getObjectsInBound({ pos.x, pos.y, wh, wh });
    }
    entity->setPosition(pos); // Set cells position to safe one

    done:
    entity->obj = Collidable({ pos.x, pos.y, wh, wh }, (Entity*)entity);
    quadTree.insert(&entity->obj); // insert into quadTree
    entities[entity->type].push_back(entity); // insert into vector of its type
    return entity;
}
template Food *spawn<Food>(Vector2 &pos, double radius, const Color &color) noexcept;
template Virus *spawn<Virus>(Vector2 &pos, double radius, const Color &color) noexcept;
template Ejected *spawn<Ejected>(Vector2 &pos, double radius, const Color &color) noexcept;
template PlayerCell *spawn<PlayerCell>(Vector2 &pos, double radius, const Color &color) noexcept;
template MotherCell *spawn<MotherCell>(Vector2 &pos, double radius, const Color &color) noexcept;

void despawn(Entity *entity) {
    std::vector<Entity*> *vec = &entities[entity->type];
    const auto index = std::find(vec->begin(), vec->end(), entity);

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

void update(unsigned long long tick) {
    // Update entities
    for (std::vector<Entity*> entityVec : entities) {
        for (Entity *entity : entityVec) {
            entity->update(tick);
            updateObject(entity);
        }
    }
    // Update playercells
    for (PlayerCell::Entity *playerCell : entities[CellType::PLAYERCELL]) {
        // Playercells should be the only entities that check for collision
        for (Collidable *obj : quadTree.getObjectsInBound(playerCell->obj.bound)) {
            handleCollision(playerCell, std::any_cast<Entity*>(obj->data));
        }
    }
}

void updateObject(Entity *entity) {
    Rect *bounds = &entity->obj.bound;
    const double radius = entity->getRadius() * 2;
    const Vector2 newPos = entity->getPosition();

    // Check if radius and position are the same as before
    if (newPos == Vector2(bounds->x(), bounds->y()) && bounds->width() == radius) {
        entity->needsUpdate = false;
        return;
    }
    bounds->update(newPos.x, newPos.y, radius, radius);
    quadTree.update(&entity->obj);
    entity->needsUpdate = true;
}

void handleCollision(Entity *predator, Entity *prey) {
    // If predator is smaller, it becomes the prey
    if (predator->getRadius() <= prey->getRadius() * 1.15) {
        predator = prey;
        prey = predator;
    }
    // Handle collisions with self
    if (predator->owner == prey->owner) {
        return;
    }
    predator->consume(prey);
}

void clear() {
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

} // namespace map