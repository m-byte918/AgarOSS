#pragma once
#include "../Entities/Entity.hpp"

class Game;

namespace map {

void init(Game *_game);
void cleanup();

const Rect &bounds() noexcept;

template <typename T>
e_ptr &spawn(const Vec2 &pos, float radius, const Color &color) noexcept;

template <typename T>
e_ptr &spawnUnsafe(const Vec2 &pos, float radius, const Color &color) noexcept;

void despawn(Entity *entity) noexcept;

void update(unsigned long long tick);

void updateObject(Entity *entity);
void resolveCollision(e_ptr cell1, e_ptr cell2) noexcept;

extern std::map<unsigned long long, e_ptr> acceleratingEntities;
extern std::vector<std::vector<e_ptr>> entities;

extern QuadTree quadTree;
extern Game *game;

} // namespace map

