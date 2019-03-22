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
e_ptr &spawn(const Vec2 &pos, float radius, const Color &color) noexcept {
    e_ptr entity = std::make_shared<T>(pos, radius, color); // Initial
    entity->shared = entity;
    const float r = radius * 2; // Width/height of circular shape

    // Can spawn near any type, return as-is
    if (entity->avoidSpawningOn == nothing) goto safe;

    {
        // Get entities near this one
        std::vector<Collidable*> found = quadTree.getObjectsInBound({ pos.x, pos.y, r, r });
        if (found.empty()) goto safe; // Safe

        // (experimental) attempts=size of the vector the entity belongs to
        const size_t attempts = entities[entity->type].size();

        // Get safe position
        Vec2 safePos = pos;
        for (size_t i = 0; i < attempts && std::any_of(found.begin(), found.end(), [&](Collidable *obj) {
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
    return entity->shared;
}
template e_ptr &spawn<Food>(const Vec2 &pos, float radius, const Color &color) noexcept;
template e_ptr &spawn<Virus>(const Vec2 &pos, float radius, const Color &color) noexcept;
template e_ptr &spawn<Ejected>(const Vec2 &pos, float radius, const Color &color) noexcept;
template e_ptr &spawn<PlayerCell>(const Vec2 &pos, float radius, const Color &color) noexcept;
template e_ptr &spawn<MotherCell>(const Vec2 &pos, float radius, const Color &color) noexcept;

template <typename T>
e_ptr &spawnUnsafe(const Vec2 &pos, float radius, const Color &color) noexcept {
    e_ptr entity = std::make_shared<T>(pos, radius, color);
    entity->shared = entity;
    const float r = radius * 2; // Width/height of circular shape

    entity->obj = Collidable({ pos.x, pos.y, r, r }, entity.get());
    entity->setBirthTick(game);
    quadTree.insert(&entity->obj); // insert into quadTree
    entities[entity->type].push_back(entity->shared); // insert into vector of its type
    return entity->shared;
}
template e_ptr &spawnUnsafe<Food>(const Vec2 &pos, float radius, const Color &color) noexcept;
template e_ptr &spawnUnsafe<Virus>(const Vec2 &pos, float radius, const Color &color) noexcept;
template e_ptr &spawnUnsafe<Ejected>(const Vec2 &pos, float radius, const Color &color) noexcept;
template e_ptr &spawnUnsafe<PlayerCell>(const Vec2 &pos, float radius, const Color &color) noexcept;
template e_ptr &spawnUnsafe<MotherCell>(const Vec2 &pos, float radius, const Color &color) noexcept;

void despawn(Entity *entity) noexcept {
    if (entity == nullptr) {
        logVerbose(Entity == nullptr!);
        return;
    }
    if (entity->state & isRemoved) {
        logVerbose(Entity is already removed!);
        return;
    }
    if (!quadTree.remove(&entity->obj)) {
        Logger::fatal("Entity could not be removed from quadTree!");
        return;
    }
    // Erase from vector of its type
    std::vector<e_ptr> &vec = entities[entity->type];
    std::vector<e_ptr>::iterator index = std::find(vec.begin(), vec.end(), entity->shared);

    if (index != vec.end())
        entities[entity->type].erase(index);

    if (entity->acceleration() > 0)
        entity->decelerate();   // Remove from accelerating entities
    entity->state |= isRemoved; // Mark as removed
    entity->onDespawned();      // Special onDespawned event
    entity->shared.reset();     // Remove last reference of shared pointer
    entity->obj.data = nullptr; // Just to be safe
}

// Update entities
void update(unsigned long long tick) {
    // Update food
    for (e_ptr &food : entities[CellType::FOOD]) {
        if (food->state & isRemoved) continue;
        food->update(tick);
        updateObject(food.get());
    }
    // Update ejected
    for (e_ptr &ejected : entities[CellType::EJECTED]) {
        if (ejected) updateObject(ejected.get());
    }
    // Update viruses
    for (e_ptr &virus : entities[CellType::VIRUS]) {
        updateObject(virus.get());
    }
    // Update playercells
    for (e_ptr &p : entities[CellType::PLAYERCELL]) {
        e_ptr playerCell = p;
        if (!playerCell || playerCell->state & isRemoved)
            continue;
        updateObject(playerCell.get());
        playerCell->update(tick);
        playerCell->autoSplit();
        for (Collidable *obj : quadTree.getObjectsInBound(playerCell->obj.bound)) {
            Entity *e = std::any_cast<Entity*>(obj->data);
            
            if (playerCell && !playerCell->acceleration())
                playerCell->collideWith(e);
        }
    }
    for (auto it = acceleratingEntities.begin(); it != acceleratingEntities.end(); ) {
        e_ptr entity = it->second;
        if (!entity || entity->state & isRemoved || !entity->decelerate()) {
            it = acceleratingEntities.erase(it);
            continue;
        }
        for (Collidable *obj : quadTree.getObjectsInBound(entity->obj.bound)) {
            Entity *e = std::any_cast<Entity*>(obj->data);
            if (e->shared && !(e->state & isRemoved))
                entity->collideWith(e);
        }
        it++;
    }
}

// Update entity's bounds if necessary
void updateObject(Entity *entity) {
    if (!entity->shared) {
        logVerbose(Shared pointer for entity is empty!);
        return;
    }
    if (entity->state & isRemoved || !(entity->state & needsUpdate))
        return;

    const float wh = entity->radius() * 2.0f;
    
    entity->obj.bound.update(entity->position().x, entity->position().y, wh, wh);
    if (!quadTree.update(&entity->obj)) {
        logVerbose(Entity could not be updated!);
        Logger::debug(entity->toString());
        despawn(entity);
        return;
    }
    if (!entity->shared) {
        logVerbose(Shared pointer for entity is empty!);
        return;
    }
    entity->state &= ~needsUpdate;
}

void resolveCollision(e_ptr cell1, e_ptr cell2) noexcept {
    // check distance between cells
    Vec2 mtd = (cell1->position() - cell2->position()).round();
    float r = cell1->radius() + cell2->radius();
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
    float impulse1 = 1 / cell1->mass();
    float impulse2 = 1 / cell2->mass();
    float impulseSum = impulse1 + impulse2;

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

void cleanup() {
    Logger::warn("Clearing Map...");

    game->commands.despawn({ "all" });
    quadTree.clear();
}

} // namespace map