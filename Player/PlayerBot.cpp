#include "PlayerBot.hpp"
#include "../Game/Map.hpp"
#include "../Game/Game.hpp"
#include "../Entities/Food.hpp"
#include "../Entities/Virus.hpp"
#include "../Entities/Ejected.hpp"
#include "../Entities/MotherCell.hpp"
#include "../Modules/Logger.hpp"

PlayerBot::PlayerBot(Server *_server) :
    Player(_server) {
}

void PlayerBot::update() {
    if (_state == PlayerState::DEAD) {
        spawn();
        updateCenter();
        updateViewBox();
    }
    else if (_state == PlayerState::PLAYING) {
        updateScore();
        updateCenter();
        updateViewBox();
        updateVisibleNodes();
        if (splitCooldown > 0)
            --splitCooldown;
        decide(*std::max_element(cells.begin(), cells.end(), [](sptr<PlayerCell::Entity> a, sptr<PlayerCell::Entity> b) {
            return a->mass() < b->mass();
        }));
    }
}
void PlayerBot::updateVisibleNodes() {
    visibleNodes.clear();
    for (Collidable *obj : map::quadTree.getObjectsInBound(viewBox)) {
        if (!obj->data.has_value()) continue;
        e_ptr entity = std::any_cast<e_ptr>(obj->data);
        if (entity && entity->owner() != this) 
            visibleNodes.push_back(entity);
    }
}
void PlayerBot::decide(sptr<PlayerCell::Entity> largestCell) {
    if (!largestCell || largestCell->state & isRemoved)
        return;
    Vec2 result{ 0, 0 };
    std::vector<e_ptr> threats;
    e_ptr splitTarget = nullptr;
    bool isPlayerInViewBox = false;

    for (e_ptr entity : visibleNodes) {
        // Get attraction of the cells - avoid larger cells, viruses and same team cells
        float influence = 0.0f;
        if (entity->type == PlayerCell::TYPE) {
            isPlayerInViewBox = true;
            if (largestCell->radius() > (entity->radius() + 4) * cfg::entity_minEatSizeMult) {
                // Can eat it
                influence = entity->radius();
            } else if (entity->radius() + 4 > largestCell->radius() * cfg::entity_minEatSizeMult) {
                // Can eat me
                influence = -entity->radius();
            } else {
                influence = -(entity->radius() / largestCell->radius()) / 3.0f;
            }
        } else if (entity->type == Food::TYPE) {
            if (largestCell->radius() > entity->radius() * cfg::entity_minEatSizeMult)
                influence = entity->radius();
        } else if (entity->type == Virus::TYPE || entity->type == MotherCell::TYPE) {
            // Virus/Mothercell
            if (largestCell->radius() > entity->radius() * cfg::entity_minEatSizeMult) {
                // Can eat it
                if (!isPlayerInViewBox || cells.size() == cfg::player_maxCells) {
                    // Won't explode
                    influence = entity->radius();
                } else {
                    // Can explode
                    influence = -entity->radius();
                }
            } else if (entity->type == MotherCell::TYPE && 
                entity->radius() > largestCell->radius() * cfg::entity_minEatSizeMult) {
                // can eat me
                influence = -entity->radius();
            }
        } else if (entity->type == Ejected::TYPE) {
            if (largestCell->radius() > entity->radius() * cfg::entity_minEatSizeMult)
                // can eat
                influence = entity->radius();
        }
        // Apply influence if it isn't 0
        if (influence == 0.0f) continue;

        // Calculate separation between cell and check
        Vec2 displacement = entity->position() - largestCell->position();

        // Figure out distance between cells
        double distance = displacement.length();
        if (influence < 0) {
            // Get edge distance
            distance -= largestCell->radius() + entity->radius();
            if (entity->type == PlayerCell::TYPE)
                threats.push_back(entity);
        }
        // The farther they are the smaller influnce it is
        if (distance < 1.0) 
            distance = 1.0; // Avoid NaN and positive influence with negative distance & attraction
        influence /= (float)distance;

        displacement *= 1.0 / displacement.length();
        displacement *= influence;
        Vec2 force = displacement;

        // Splitting conditions
        if (entity->type == PlayerCell::TYPE &&
            largestCell->radius() * INV_SQRT_2 > entity->radius() * cfg::entity_minEatSizeMult &&
            largestCell->mass() < entity->mass() * 25 &&
            splitCooldown == 0 && 
            cells.size() < 3) {

            double endDist = cfg::playerCell_initialAcceleration + cfg::game_timeStep - 
                largestCell->radius() * INV_SQRT_2 - entity->radius();

            if (endDist > 0 && distance < endDist)
                splitTarget = entity;
        } else {
            result += force;
        }
    }
    result.normalize();

    if (splitTarget != nullptr) {
        if (threats.size() > 0) {
            if (largestCell->radius() > (*std::max_element(threats.begin(), threats.end(), [](e_ptr a, e_ptr b) {
                return a->radius() < b->radius();
            }))->radius() * 1.15) {
                _mouse = splitTarget->position();
                splitCooldown = 16;
                onSplit();
            }
        } else {
            _mouse = splitTarget->position();
            splitCooldown = 16;
            onSplit();
        }
    }
    _mouse = {
        largestCell->position().x + result.x * 800.0,
        largestCell->position().y + result.y * 800.0
    };
}

PlayerBot::~PlayerBot() {
}