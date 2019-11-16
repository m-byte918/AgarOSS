#include "Food.hpp"
#include "../Game/Map.hpp"
#include "../Game/Game.hpp" // configs

Food::Food(const Vec2 &pos, float radius, const Color &color) noexcept :
    Entity(pos, radius, color) {

    flag = food;
    canEat = cfg::food_canEat;
    avoidSpawningOn = cfg::food_avoidSpawningOn;

    if (cfg::food_isSpiked)   state |= isSpiked;
    if (cfg::food_isAgitated) state |= isAgitated;
}
void Food::update() noexcept {
    if (!cfg::food_canGrow || _radius >= cfg::food_maxRadius) 
        return;

    // 10% chance to grow every minute
    if (++growTick > 1500) {
        if (rand(0, 10) == 10)
            setMass(_mass + 1); // setMass might be faster in this case
        growTick = 0;
    }
}
void Food::onDespawned() noexcept {
    // Vanilla servers spawn new food as soon as one is eaten, so lets do that
    if (map::entities[Food::TYPE].size() < cfg::food_startAmount)
        map::spawn<Food>(randomPosition(), cfg::food_baseRadius, randomColor());
}
Food::~Food() {
}