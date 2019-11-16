#pragma once
#include "../Entities/Entity.hpp"

class Game;

namespace map {

void init(Game *_game);
void cleanup();

const Rect &bounds() noexcept;

template <typename T>
sptr<T> spawn(Vec2 pos, float radius, const Color &color, bool checkSafe = true) noexcept;

void despawn(e_ptr entity) noexcept;

void update();

void resolveCollision(e_ptr cell1, e_ptr cell2) noexcept;

extern std::vector<e_ptr> movingEntities;
extern std::vector<std::vector<e_ptr>> entities;

extern QuadTree quadTree;
extern Game *game;
extern float dt;

} // namespace map