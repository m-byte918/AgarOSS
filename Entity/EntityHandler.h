#include <iostream>
#include <algorithm>
#include "Entity.h"
#include "../Settings.h"

std::vector<Food*>       _food;        // Vector of food
std::vector<Virus*>      _viruses;     // Vector of viruses
std::vector<MotherCell*> _mothercells; // Vector of mothercells
std::vector<PlayerCell*> _playercells; // Vector of playercells

namespace ent {

    // Ent *ent1 = ent::add<Ent>(_vec, {x, y}, {r, g, b}, size);
    // adds a unique entity and returns a pointer to it
    template <class T>
    T* add(std::vector<auto*> &_vec, const Position& position, const Color& color, const float& size) {
        T *entity = new T();
        entity->setSize(size);
        entity->setColor(color);
        entity->setPosition(position);
        _vec.push_back(entity);
        return entity;
    }

    // todo: move this to Util.h without creating circular dependency
    // returns random position on the map through format { x, y }
    Position getRandomPosition() {
        return {
            (float)rand(-cfg::mapWidth / 2, cfg::mapWidth / 2), 
            (float)rand(-cfg::mapHeight / 2, cfg::mapHeight / 2)
        };
    }

    void spawnStartingEntities() {
        for (int i = 0; i <= cfg::foodStartAmount; ++i) {
            add<Food>(_food, getRandomPosition(), randomColor(), cfg::foodStartSize);
        }
        for (int i = 0; i <= cfg::virusStartAmount; ++i) {
            add<Virus>(_viruses, getRandomPosition(), cfg::virusColor, cfg::virusStartSize);
        }
    }

    // template this function for multiple entity-use l8ter
    PlayerCell* addSafePlayerCell(const Color& color, const float& size) {
        Position position = getRandomPosition();

        // Check for intersection with other players and viruses
        // todo: replace infinite while loop with a set amount of attempts
        for (const auto &i : _playercells) {
            while (checkIntersection(position, size, i) == true) {
                position = getRandomPosition();
                continue;
            }
        }
        for (const auto &i : _viruses) {
            while (checkIntersection(position, size, i) == true) {
                std::cout << "yes2\n";
                position = getRandomPosition();
                continue;
            }
        }
        return add<PlayerCell>(_playercells, position, color, size);
    }
    //MotherCell* addSafeMotherCell() {
    //}
    //Virus* addSafeVirus() {
    //}

    // ent::remove(entity, vector);
    // deletes entity and removes it from vector
    void remove(auto &entity, auto &vec) {
        delete entity;
        entity = nullptr;
        vec.erase(std::remove(vec.begin(), vec.end(), entity), vec.end());
    }

    // removes all entities and clears all vectors
    void removeAll() {
        // remove all entities
        for (int i = 0; i < _food.size(); ++i)
            remove(_food[i], _food);
        for (int i = 0; i < _viruses.size(); ++i)
            remove(_viruses[i], _viruses);
        for (int i = 0; i < _mothercells.size(); ++i)
            remove(_mothercells[i], _mothercells);
        for (int i = 0; i < _playercells.size(); ++i)
            remove(_playercells[i], _playercells);

        // clear all entity vectors
        _food.clear();
        _viruses.clear();
        _mothercells.clear();
        _playercells.clear();
    }
}
