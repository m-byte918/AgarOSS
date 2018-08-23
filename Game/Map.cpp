#include "Map.hpp"
#include "../Player.hpp"
#include "../Game/Game.hpp"
#include "../Modules/Logger.hpp"
#include "../Entities/Food.hpp"
#include "../Entities/Virus.hpp"
#include "../Entities/Ejected.hpp"
#include "../Entities/MotherCell.hpp"
#include "../Entities/PlayerCell.hpp"

namespace map {

std::vector<std::vector<e_ptr>> entities{
    std::vector<std::shared_ptr<Food::Entity>>(),
    std::vector<std::shared_ptr<Virus::Entity>>(),
    std::vector<std::shared_ptr<Ejected::Entity>>(),
    std::vector<std::shared_ptr<MotherCell::Entity>>(),
    std::vector<std::shared_ptr<PlayerCell::Entity>>()
};

QuadTree quadTree;

void init() {
    Logger::info("Initializing Map...");

    quadTree = QuadTree({
        0,
        0,
        cfg::game_mapWidth,
        cfg::game_mapHeight
        }, cfg::game_quadTreeLeafCapacity, cfg::game_quadTreeMaxDepth);

    // Spawn starting food
    while (entities[CellType::FOOD].size() < cfg::food_startAmount)
        spawn<Food>(randomPosition(), cfg::food_baseRadius, randomColor());

    // Spawn starting viruses
    while (entities[CellType::VIRUS].size() < cfg::virus_startAmount)
        spawn<Virus>(randomPosition(), cfg::virus_baseRadius, cfg::virus_color);
}

const Rect &getBounds() noexcept {
    return quadTree.getBounds();
}

template <typename T>
e_ptr spawn(const Vector2 &pos, double radius, const Color &color) noexcept {
    std::shared_ptr<Entity> entity = std::make_shared<T>(pos, radius, color); // Initial
    const double r = radius * 2; // Width/height of circular shape

    // Can spawn near any type, return as-is
    if (entity->avoidSpawningOn == nothing) goto safe;

    {
        // Get entities near this one
        std::vector<Collidable*> found = quadTree.getObjectsInBound({ pos.x, pos.y, r, r });
        if (found.empty()) goto safe; // Safe

        // (experimental) attempts=size of the vector the entity belongs to
        const unsigned attempts = entities[entity->type].size();

        // Get safe position
        Vector2 safePos = pos;
        for (unsigned i = 0; i < attempts && std::any_of(found.begin(), found.end(), [&](Collidable *obj) {
            const e_ptr &cell = std::any_cast<e_ptr>(obj->data);
            return (entity->avoidSpawningOn & cell->flag) && cell->intersects(safePos, radius);
        }); ++i) {
            // Retry
            safePos = randomPosition();
            found = quadTree.getObjectsInBound({ safePos.x, safePos.y, r, r });
        }
        entity->setPosition(safePos); // Set cells position to safe one
    }

    safe:
    entity->obj = Collidable({ pos.x, pos.y, r, r }, entity);
    quadTree.insert(&entity->obj); // insert into quadTree
    entities[entity->type].push_back(entity); // insert into vector of its type
    return entity;
}
template e_ptr spawn<Food>(const Vector2 &pos, double radius, const Color &color) noexcept;
template e_ptr spawn<Virus>(const Vector2 &pos, double radius, const Color &color) noexcept;
template e_ptr spawn<Ejected>(const Vector2 &pos, double radius, const Color &color) noexcept;
template e_ptr spawn<PlayerCell>(const Vector2 &pos, double radius, const Color &color) noexcept;
template e_ptr spawn<MotherCell>(const Vector2 &pos, double radius, const Color &color) noexcept;

void despawn(const e_ptr &entity) {
    // Remove from quadTree if it exists
    if (!quadTree.remove(&entity->obj)) {
        Logger::error("Cannot remove entity that does not exist!");
        return;
    }
    // Erase from vector of its type
    std::vector<e_ptr> *vec = &entities[entity->type];
    vec->erase(std::find(vec->begin(), vec->end(), entity));

    entity->isRemoved = true; // Mark as removed
    entity->onDespawned();    // Special onDespawned event
}

void update(unsigned long long tick) {
    // Update entities
    for (std::vector<e_ptr> entityVec : entities) {
        for (e_ptr entity : entityVec) {
            entity->update(tick);

            // Update object bounds if necessary
            if (entity->needsUpdate) {
                updateObject(entity);
            }
            // Decelerate entity if necessary
            if (entity->isAccelerating()) {
                for (Collidable *obj : quadTree.getObjectsInBound(entity->obj.bound)) {
                    handleCollision(entity, std::any_cast<e_ptr>(obj->data));
                }
                entity->decelerate();
            }
        }
    }
    // Update playercells
    for (e_ptr playerCell : entities[CellType::PLAYERCELL]) {
        playerCell->needsUpdate = true; // ?
        for (Collidable *obj : quadTree.getObjectsInBound(playerCell->obj.bound)) {
            handleCollision(playerCell, std::any_cast<e_ptr>(obj->data));
        }
    }
}

void updateObject(const e_ptr &entity) {
    const double wh = entity->getRadius() * 2;
    const Vector2 &pos = entity->getPosition();

    entity->obj.bound.update(pos.x, pos.y, wh, wh);
    quadTree.update(&entity->obj);
    entity->needsUpdate = false;
}

void handleCollision(const e_ptr &cell1, const e_ptr &cell2) {
    if (!cell1->intersects(cell2)) return;

    // Handle everything but eat collisions first
    if (cell1->getCreator() != 0) {
        if (cell1->getCreator() == cell2->getCreator()) {
            if (cell1->type == cell2->type) {
                resolveCollision(cell1, cell2);
                return;
            }
            //handleCollisionWithCreator()
            return;
        }
    }
    e_ptr predator = cell1;
    e_ptr prey = cell2;
    // If predator is smaller, it becomes the prey
    if (predator->getRadius() <= prey->getRadius() * cfg::entity_minEatSizeMult) {
        predator = cell2;
        prey = cell1;
    }
    predator->consume(prey);
}

void resolveCollision(const e_ptr &cell1, const e_ptr &cell2) noexcept {
    // get minimum translation distance between cells
    Vector2 delta = cell1->getPosition() - cell2->getPosition();
    double r = cell1->getRadius() + cell2->getRadius();
    double dist = delta.squared();

    if (dist > r * r) return; // No collision
    dist = std::sqrt(dist);

    // Special case (cells are exactly on top of eachother)
    Vector2 mtd;
    if (dist == 0.0) {
        dist = r - 1.0;
        delta = Vector2(r, 0.0);
    }
    mtd = delta * ((r - dist) / dist);

    // resolve intersection
    double impulse1 = 1 / cell1->getMass();
    double impulse2 = 1 / cell2->getMass();
    double impulseSum = impulse1 + impulse2;

    // push-pull them apart based off their mass
    cell1->setPosition(cell1->getPosition() + (mtd * (impulse1 / impulseSum)), true);
    cell2->setPosition(cell2->getPosition() - (mtd * (impulse2 / impulseSum)), true);

    //// impact speed
    //Vector2 v = cell1->velocity - cell2->velocity;
    //mtd.normalize();
    //double vn = v.x * mtd.x + v.y * mtd.y;

    //// cells intersect but are moving away from each other already
    //if (vn > 0.0f) return;

    //// collision impulse
    //// restitution coefficient: 0.85
    //// could also just be -2.0f * vn (test this later)
    //double ligma = (-(1.0f + 0.85) * vn) / impulseSum;
    //Vector2 impulse = mtd * ligma;

    //// change in momentum
    //cell1->velocity += (impulse * impulse1);
    //cell2->velocity -= (impulse * impulse2);
}

void clear() {
    Logger::warn("Clearing Map...");

    quadTree.clear();
    for (std::vector<e_ptr> &entityVec : entities) {
        Logger::logDebug(entityVec.size());
        for (const e_ptr &entity : entityVec) {
            //despawn(entity);
        }
        entityVec.clear();
    }
}

} // namespace map