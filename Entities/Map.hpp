#pragma once
#include "Entities.hpp"

class Map {
public:
    void init();
    void clear();

    static const Rect &getBounds() noexcept;

    static Food *spawnFood(const Vector2& = getRandomPosition(), double = config["food"]["startSize"], 
        const Color& = getRandomColor()) noexcept;

    static Virus *spawnVirus(const Vector2&, double = config["virus"]["startSize"], 
        const Color& = config["virus"]["color"]) noexcept;
    static Virus *spawnVirus(double = config["virus"]["startSize"], const Color& = config["virus"]["color"]) noexcept;

    static Ejected *spawnEjected(const Vector2& = getRandomPosition(), double = config["ejected"]["startSize"], 
        const Color& = getRandomColor()) noexcept;

    static MotherCell *spawnMotherCell(const Vector2&, double = config["motherCell"]["startSize"], 
        const Color& = config["motherCell"]["color"]) noexcept;
    static MotherCell *spawnMotherCell(double = config["motherCell"]["startSize"],  
        const Color& = config["motherCell"]["color"]) noexcept;

    static PlayerCell *spawnPlayerCell(const Vector2&, double = config["playerCell"]["startSize"], 
        const Color& = getRandomColor()) noexcept;
    static PlayerCell *spawnPlayerCell(double = config["playerCell"]["startSize"], const Color& = getRandomColor()) noexcept;

    static void removeEntity(Entity *entity);

    static void updateObject(Entity *entity);

    void update(unsigned long long tick);
    
    static std::vector<std::vector<Entity*>> entities;
    static QuadTree quadTree;

private:
    static Vector2 getSafePosition(double size) noexcept;

    template <typename T>
    static T *spawnEntity(const Vector2 &pos, double size, const Color &color) noexcept;
};

