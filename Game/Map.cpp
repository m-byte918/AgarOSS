#include "Map.hpp"
#include "../Game/Game.hpp"
#include "../Player/Player.hpp"
#include "../Modules/Logger.hpp"
#include "../Entities/Food.hpp"
#include "../Entities/Virus.hpp"
#include "../Entities/Ejected.hpp"
#include "../Entities/MotherCell.hpp"
#include "../Entities/PlayerCell.hpp"

namespace map {

std::map<unsigned long long, e_ptr> acceleratingEntities{};
std::vector<std::vector<e_ptr>> entities{
    std::vector<std::shared_ptr<Food::Entity>>(),
    std::vector<std::shared_ptr<Virus::Entity>>(),
    std::vector<std::shared_ptr<Ejected::Entity>>(),
    std::vector<std::shared_ptr<MotherCell::Entity>>(),
    std::vector<std::shared_ptr<PlayerCell::Entity>>()
};

Game *game;
QuadTree quadTree;

void init(Game *_game) {
    Logger::print("\n");
    Logger::info("Initializing Map...");

    game = _game;
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

const Rect &bounds() noexcept {
    return quadTree.getBounds();
}

template <typename T>
e_ptr spawn(const Vec2 &pos, double radius, const Color &color) noexcept {
    e_ptr entity = std::make_shared<T>(pos, radius, color); // Initial
    entity->shared = entity;
    const double r = radius * 2; // Width/height of circular shape

    // Can spawn near any type, return as-is
    if (entity->avoidSpawningOn == nothing) goto safe;

    {
        // Get entities near this one
        std::vector<Collidable*> found = quadTree.getObjectsInBound({ pos.x, pos.y, r, r });
        if (found.empty()) goto safe; // Safe

        // (experimental) attempts=size of the vector the entity belongs to
        const unsigned int attempts = (unsigned int)entities[entity->type].size();

        // Get safe position
        Vec2 safePos = pos;
        for (unsigned i = 0; i < attempts && std::any_of(found.begin(), found.end(), [&](Collidable *obj) {
            Entity *cell = std::any_cast<Entity*>(obj->data);
            return (entity->avoidSpawningOn & cell->flag) && cell->intersects(safePos, radius);
        }); ++i) {
            // Retry
            safePos = randomPosition();
            found = quadTree.getObjectsInBound({ safePos.x, safePos.y, r, r });
        }
        entity->setPosition(safePos); // Set cells position to safe one
    }

safe:
    entity->obj = Collidable({ pos.x, pos.y, r, r }, entity.get());
    entity->setBirthTick(game);
    quadTree.insert(&entity->obj); // insert into quadTree
    entities[entity->type].push_back(entity->shared); // insert into vector of its type
    return entity;
}
template e_ptr spawn<Food>(const Vec2 &pos, double radius, const Color &color) noexcept;
template e_ptr spawn<Virus>(const Vec2 &pos, double radius, const Color &color) noexcept;
template e_ptr spawn<Ejected>(const Vec2 &pos, double radius, const Color &color) noexcept;
template e_ptr spawn<PlayerCell>(const Vec2 &pos, double radius, const Color &color) noexcept;
template e_ptr spawn<MotherCell>(const Vec2 &pos, double radius, const Color &color) noexcept;

template <typename T>
e_ptr spawnUnsafe(const Vec2 &pos, double radius, const Color &color) noexcept {
    e_ptr entity = std::make_shared<T>(pos, radius, color);
    entity->shared = entity;
    const double r = radius * 2; // Width/height of circular shape

    entity->obj = Collidable({ pos.x, pos.y, r, r }, entity.get());
    entity->setBirthTick(game);
    quadTree.insert(&entity->obj); // insert into quadTree
    entities[entity->type].push_back(entity->shared); // insert into vector of its type
    return entity;
}
template e_ptr spawnUnsafe<Food>(const Vec2 &pos, double radius, const Color &color) noexcept;
template e_ptr spawnUnsafe<Virus>(const Vec2 &pos, double radius, const Color &color) noexcept;
template e_ptr spawnUnsafe<Ejected>(const Vec2 &pos, double radius, const Color &color) noexcept;
template e_ptr spawnUnsafe<PlayerCell>(const Vec2 &pos, double radius, const Color &color) noexcept;
template e_ptr spawnUnsafe<MotherCell>(const Vec2 &pos, double radius, const Color &color) noexcept;

void despawn(e_ptr &entity) noexcept {
    if (!entity) {
        logVerbose(Shared pointer for entity is empty!);
        return;
    }
    if (entity->isRemoved == true) {
        logVerbose(Entity is already removed!);
        return;
    }
    // Erase from vector of its type
    std::vector<e_ptr> &vec = entities[entity->type];
    std::vector<e_ptr>::iterator index = std::remove(vec.begin(), vec.end(), entity->shared);

    if (index == vec.end()) {
        Logger::warn(entity->toString());
        return;
    }
    entities[entity->type].erase(index);
    // Remove from quadTree
    if (!quadTree.remove(&entity->obj)) {
        logVerbose(Entity could not be removed from quadTree!);
        return;
    }
    entity->isRemoved   = true;      // Mark as removed
    entity->needsUpdate = true;      // Last update before it is removed from client
    if (entity->acceleration() > 0)
        entity->decelerate();        // Remove from accelerating entities
    entity->onDespawned();           // Special onDespawned event
}

// Update entities
void update(unsigned long long tick) {
    // Update food
    for (e_ptr &food : entities[CellType::FOOD]) {
        if (food->isRemoved) continue;
        food->update(tick);
        updateObject(food.get());
    }
    // Update ejected
    std::vector<e_ptr> &evec = entities[CellType::EJECTED];
    for (e_ptr &ejected : evec) {
        if (!ejected) {
            logVerbose(Shared pointer for ejected mass is empty! Removing from vector.);
            std::vector<e_ptr>::iterator i = std::find(evec.begin(), evec.end(), ejected);
            if (i != evec.end())
                evec.erase(i);
            continue;
        }
        // Read access violation here ?
        if (ejected->isRemoved) continue;
        updateObject(ejected.get());
    }
    // Update viruses
    for (e_ptr &virus : entities[CellType::VIRUS]) {
        if (virus->isRemoved) continue;
        updateObject(virus.get());
    }
    // Update playercells
    std::vector<e_ptr> &pvec = entities[CellType::PLAYERCELL];
    for (e_ptr &playerCell : pvec) {
        if (!playerCell) {
            logVerbose(Shared pointer for playercell is empty! Removing from vector.);
            std::vector<e_ptr>::iterator i = std::remove(pvec.begin(), pvec.end(), playerCell);
            if (i != pvec.end())
                pvec.erase(i);
            continue;
        }
        if (playerCell->isRemoved) continue;
        updateObject(playerCell.get());
        playerCell->update(tick);
        for (Collidable *obj : quadTree.getObjectsInBound(playerCell->obj.bound)) {
            Entity *e = std::any_cast<Entity*>(obj->data);
            handleCollision(playerCell, e);
        }
        if (!playerCell) {
            logVerbose(Shared pointer for playercell is empty! Removing from vector.);
            std::vector<e_ptr>::iterator i = std::remove(pvec.begin(), pvec.end(), playerCell);
            if (i != pvec.end())
                pvec.erase(i);
            continue;
        }
        playerCell->autoSplit();
    }
    std::map<unsigned long long, e_ptr>::iterator begin = acceleratingEntities.begin();
    std::map<unsigned long long, e_ptr>::iterator end = acceleratingEntities.end();
    for (; begin != end; ) {
        if (!begin->second) {
            begin = acceleratingEntities.erase(begin);
            continue;
        }
        else if (begin->second->isRemoved == true) {
            begin = acceleratingEntities.erase(begin);
            continue;
        }
        else if (!begin->second->decelerate()) {
            begin = acceleratingEntities.erase(begin);
            continue;
        }
        std::vector<Collidable*> found = quadTree.getObjectsInBound(begin->second->obj.bound);

        for (Collidable *obj : found) {
            Entity *e = std::any_cast<Entity*>(obj->data);
            if (!e->shared) {
                logVerbose(Shared pointer from quadtree is empty!);
                return;
            }
            if (e->isRemoved) {
                logVerbose(Entity pulled from quadtree is removed!);
                Logger::debug(e->toString());
                return;
            }
            handleCollision(begin->second, e);
        }
        begin++;
    }
}

// Update entity's bounds if necessary
void updateObject(Entity *entity) {
    if (!entity->shared) {
        logVerbose(Shared pointer for entity is empty!);
        return;
    }
    if (!entity->needsUpdate || entity->isRemoved)
        return;

    const double wh = entity->radius() * 2;
    const Vec2 &pos = entity->position();

    entity->obj.bound.update(pos.x, pos.y, wh, wh);
    if (!quadTree.update(&entity->obj)) {
        logVerbose(Entity could not be updated!);
        despawn(entity->shared);
        Logger::debug(entity->toString());
        return;
    }
    if (!entity->shared) {
        logVerbose(Shared pointer for entity is empty!);
        return;
    }
    entity->needsUpdate = false;
}

void handleCollision(e_ptr &cell1, Entity *cell2) {
    if (!cell1 || !cell2->shared)
        return;
    if (cell1->isRemoved || cell2->isRemoved || !cell1->intersects(cell2->shared))
        return;

    // Determine if predator should become prey
    bool isPredatorSmaller = cell1->radius() <= cell2->radius() * cfg::entity_minEatSizeMult;

    // Resolve rigid collisions
    if (cell1->creator() != 0 && cell2->creator() != 0 && cell1->type == cell2->type) {
        // Ejected -> resolve collision immediately
        if (cell1->type == CellType::EJECTED) {
            resolveCollision(cell1, cell2->shared);
            // Set velocity again to start chain reaction
            if (cell2->acceleration() == 0)
                cell2->setVelocity(1, cell1->position().angle());
            return;
        }
        // Playercells from same owner
        else if (cell1->creator() == cell2->creator()) {
            if (!cell1->ignoreCollision || !cell2->ignoreCollision) {
                // Just split -> resolve collision after 15 ticks
                if (cell1->age() > cfg::player_collisionIgnoreTime &&
                    cell2->age() > cfg::player_collisionIgnoreTime) {
                    resolveCollision(cell1, cell2->shared);
                    return;
                }
                return; // Merging -> do not eat or resolve collision
            }
        }
        // Viruses -> do not eat or resolve collision
        else if (cell1->owner() == cell2->owner())
            return;
        // Playercells from different owners -> do not consume
        // if predator is smaller
        else if (isPredatorSmaller)
            return;
    }
    e_ptr predator = cell1;
    e_ptr prey = cell2->shared;
    if (isPredatorSmaller) {
        predator = prey;
        prey = cell1;
    }
    predator->consume(prey);
}

void resolveCollision(e_ptr cell1, e_ptr cell2) noexcept {
    // check distance between cells
    Vec2 mtd = (cell1->position() - cell2->position()).round();
    double r = cell1->radius() + cell2->radius();
    double dist = mtd.length();

    // Special case (cells are exactly on top of eachother)
    if (dist == 0.0) {
        dist = r - 1.0;
        mtd = Vec2(r, 0.0);
    }
    double penetration = r - dist;
    Vec2 normal = mtd / dist;
    mtd = normal * penetration; // minimum translation distance

    // Get impulses
    double impulse1 = 1 / cell1->mass();
    double impulse2 = 1 / cell2->mass();
    double impulseSum = impulse1 + impulse2;

    // Cell has infinite mass, do not move
    if (impulseSum == 0) return;

    // Momentum
    mtd /= impulseSum;

    // push-pull them apart based off their mass    
    cell1->setPosition(cell1->position() + mtd * impulse1, true);    
    cell2->setPosition(cell2->position() - mtd * impulse2, true);

    // (experimental) resolving penetration
    float percent = 0.0015f; // Percentage to move (higher= more jitter, less overlap)
    float slop = 0.001f; // If penetration is less than slop value then don't correct

    // Positional correction vector
    Vec2 correction = normal * percent * (std::max(penetration - slop, 0.0) / impulseSum);

    // Move away from each other based on correction amount
    cell1->setPosition(cell1->position() + correction * impulse1, true);
    cell2->setPosition(cell2->position() - correction * impulse2, true);
}

void clear() {
    Logger::warn("Clearing Map...");

    quadTree.clear();
    for (std::vector<e_ptr> &entityVec : entities) {
        Logger::logDebug(entityVec.size());
        for (e_ptr entity : entityVec) {
            //despawn(entity);
        }
        entityVec.clear();
    }
}

} // namespace map