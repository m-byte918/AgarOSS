#pragma once
#include "Entities.hpp"

namespace map {

void init();
void clear();

const Rect &getBounds() noexcept;

template <typename T>
T *spawn(Vector2 &pos, double radius, const Color &color) noexcept;

void despawn(Entity *entity);

void update(unsigned long long tick);

void updateObject(Entity *entity);
void handleCollision(Entity *predator, Entity *prey);

extern std::vector<std::vector<Entity*>> entities;

extern QuadTree quadTree;

} // namespace map

