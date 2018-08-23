#pragma once
#include "../Entities/Entity.hpp"

namespace map {

void init();
void clear();

const Rect &getBounds() noexcept;

template <typename T>
e_ptr spawn(const Vector2 &pos, double radius, const Color &color) noexcept;

void despawn(const e_ptr &entity);

void update(unsigned long long tick);

void updateObject(const e_ptr &entity);
void handleCollision(const e_ptr &cell1, const e_ptr &cell2);
void resolveCollision(const e_ptr &cell1, const e_ptr &cell2) noexcept;

extern std::vector<std::vector<e_ptr>> entities;

extern QuadTree quadTree;

} // namespace map

