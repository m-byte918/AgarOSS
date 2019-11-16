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

std::vector<e_ptr> movingEntities{};
std::vector<std::vector<e_ptr>> entities{
    std::vector<sptr<Food::Entity>>(),
    std::vector<sptr<Virus::Entity>>(),
    std::vector<sptr<Ejected::Entity>>(),
    std::vector<sptr<MotherCell::Entity>>(),
    std::vector<sptr<PlayerCell::Entity>>()
};

Game *game;
QuadTree quadTree;

void init(Game *_game) {
    Logger::info("Creating QuadTree...");

    game = _game;
    quadTree = QuadTree({
        0,
        0,
        cfg::game_mapWidth,
        cfg::game_mapHeight
    }, cfg::game_quadTreeLeafCapacity, cfg::game_quadTreeMaxDepth);

    // Spawn starting food
    Logger::info("Spawning ", cfg::food_startAmount, " food...");
    while (entities[Food::TYPE].size() < cfg::food_startAmount)
        spawn<Food>(randomPosition(), cfg::food_baseRadius, randomColor());

    // Spawn starting viruses
    Logger::info("Spawning ", cfg::virus_startAmount, " viruses...");
    while (entities[Virus::TYPE].size() < cfg::virus_startAmount)
        spawn<Virus>(randomPosition(), cfg::virus_baseRadius, cfg::virus_color);

    /*// Spawn starting mothercells
    Logger::info("Spawning ", cfg::motherCell_startAmount, " mothercells...");
    while (entities[MotherCell::TYPE].size() < cfg::motherCell_startAmount)
        spawn<MotherCell>(randomPosition(), cfg::motherCell_baseRadius, cfg::motherCell_color);*/

    // Spawn starting player bots
    if (cfg::server_playerBots > 0)
        game->commands.playerbot({ cfg::server_playerBots });
}

const Rect &bounds() noexcept {
    return quadTree.getBounds();
}
 
template <typename T>
sptr<T> spawn(Vec2 pos, float radius, const Color &color, bool checkSafe) noexcept {
    sptr<T> entity = std::make_shared<T>(pos, radius, color); // Initial
    entity->type   = T::TYPE;
    entity->shared = entity; // 1
    const float r  = radius * 2; // Width/height of circular shape

    // Check if entity should use safespawn
    if (checkSafe && entity->avoidSpawningOn != nothing) {
        // Get entities near this one
        std::vector<Collidable*> found = quadTree.getObjectsInBound({ pos.x, pos.y, r, r });

        // Get safe position
        int attempts = (int)entities[T::TYPE].size();
        for (;!found.empty() && attempts > 0 && std::any_of(found.begin(), found.end(), [&](Collidable *obj) {
            if (!obj->data.has_value()) return false;
            e_ptr cell = std::any_cast<e_ptr>(obj->data);
            return (entity->avoidSpawningOn & cell->flag) && cell->intersects(pos, r);
        }); --attempts) {
            // Retry
            pos = randomPosition();
            found = quadTree.getObjectsInBound({ pos.x, pos.y, r, r });
        }
    }
    entity->setPosition(pos); // Set cells position to safe one (if necessary)
    entity->obj = Collidable({ pos.x, pos.y, r, r }, entity->shared); // 2
    entity->setBirthTick(game);
    quadTree.insert(&entity->obj); // insert into quadTree
    entities[T::TYPE].push_back(entity->shared); // (3) insert into vector of its type
    return entity;
}
template sptr<Food> spawn<Food>(Vec2 pos, float radius, const Color &color, bool checkSafe) noexcept;
template sptr<Virus> spawn<Virus>(Vec2 pos, float radius, const Color &color, bool checkSafe) noexcept;
template sptr<Ejected> spawn<Ejected>(Vec2 pos, float radius, const Color &color, bool checkSafe) noexcept;
template sptr<PlayerCell> spawn<PlayerCell>(Vec2 pos, float radius, const Color &color, bool checkSafe) noexcept;
template sptr<MotherCell> spawn<MotherCell>(Vec2 pos, float radius, const Color &color, bool checkSafe) noexcept;

void despawn(e_ptr entity) noexcept {
    if (!entity || entity->state & isRemoved) {
        Logger::error("Entity is already removed.");
        return;
    }
    // Remove from quadTree
    if (!quadTree.remove(&entity->obj)) {
        Logger::error("Entity could not be removed from quadTree.");
        Logger::debug(entity->toString());
        //return;
    }
    // Erase from vector of its type
    std::vector<e_ptr> &vec = entities[entity->type];
    std::vector<e_ptr>::iterator index = std::find(vec.begin(), vec.end(), entity->shared);
    if (index == vec.end()) {
        Logger::error("Entity was not in its vector.");
        return;
    }
    vec.erase(index);
    entity->state |= isRemoved; // Mark as removed
    entity->onDespawned();      // Special onDespawned event
    entity->obj.data.reset();   // Remove reference of shared pointer in quadTree
    entity->shared.reset();     // Remove last reference of shared pointer
}

// Update entities
void update() {
    // Update food
    for (unsigned i = 0; i < entities[Food::TYPE].size(); ++i) {
        sptr<Food::Entity> food = entities[Food::TYPE][i];
        if (food && !(food->state & isRemoved))
            food->update();
    }
    // Update playercells
    for (unsigned i = 0; i < entities[PlayerCell::TYPE].size(); ++i) {
        sptr<PlayerCell::Entity> playerCell = entities[PlayerCell::TYPE][i];
        if (!playerCell || playerCell->state & isRemoved)
            continue;
        playerCell->update();
        playerCell->autoSplit();
        if (playerCell->acceleration())
            continue;
        for (Collidable *obj : quadTree.getObjectsInBound(playerCell->obj.bound)) {
            if (!playerCell || playerCell->state & isRemoved) break;
            if (!obj->data.has_value()) continue;
            playerCell->collideWith(std::any_cast<e_ptr>(obj->data));
        }
    }
    // Update moving entities
    for (int i = (int)movingEntities.size() - 1; i >= 0; --i) {
        e_ptr entity = movingEntities[i];
        if (!entity || entity->state & isRemoved || !entity->decelerate()) {
            movingEntities.erase(movingEntities.begin() + i);
            continue;
        }
        for (Collidable *obj : quadTree.getObjectsInBound(entity->obj.bound)) {
            if (!entity || entity->state & isRemoved) break;
            if (!obj->data.has_value()) continue;
            entity->collideWith(std::any_cast<e_ptr>(obj->data));
        }
    }
}

void resolveCollision(e_ptr A, e_ptr B) noexcept {
    // Check distance between cells
    Vec2 mtd = (A->position() - B->position()).round();
    float r = A->radius() + B->radius();
    double dist = mtd.length();

    // Special case (cells are exactly on top of eachother)
    if (dist == 0.0) {
        dist = r - 1.0;
        mtd = Vec2(r, 0.0);
    }
    // Get minimum translation distance
    double penetration = r - dist;
    Vec2 normal = mtd / dist;
    mtd = normal * penetration;

    // Get impulses
    float impulseSum = A->invMass() + B->invMass();

    // Cell has infinite mass, do not move
    if (impulseSum == 0) return;

    // Momentum
    mtd /= impulseSum;

    // push-pull them apart based off their mass    
    A->setPosition(A->position() + mtd * A->invMass(), true);
    B->setPosition(B->position() - mtd * B->invMass(), true);

    // (experimental) resolving penetration
    float percent = 0.0015f; // Percentage to move (higher= more jitter, less overlap)
    float slop = 0.001f; // If penetration is less than slop value then don't correct

    // Positional correction vector
    Vec2 correction = normal * percent * (std::max(penetration - slop, 0.0) / impulseSum);

    // Move away from each other based on correction amount
    A->setPosition(A->position() + correction * A->invMass(), true);
    B->setPosition(B->position() - correction * B->invMass(), true);
}

void cleanup() {
    Logger::warn("Clearing Map...");

    game->commands.despawn({ "all" });
    quadTree.clear();
}

} // namespace map