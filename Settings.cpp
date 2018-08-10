#include "Modules/Utils.hpp"

json config = {
    { "server", {
        { "port", 8080 },
        { "name", "MultiOgar-Cpp" },
        { "maxConnections", 500 },
        { "maxSupportedProtocol", 18 },
        { "minSupportedProtocol", 1 }
    }},

    { "game", {
        { "mode", 0 },
        { "timeStep", 40 },
        { "mapWidth", 14142.135623730952 },
        { "mapHeight", 14142.135623730952 },
        { "quadTreeLeafCapacity", 64 },
        { "quadTreeMaxDepth", 32 }
    }},

    { "player", {
        { "maxNameLength", 15 },
        { "maxCells", 16 },
        { "minViewBoxScale", 0.15 },
        { "viewBoxWidth", 1920 },
        { "viewBoxHeight", 1080 }
    }},

    // to get radius from mass, use toRadius(mass)

    { "playerCell", {
        { "baseRadius", 31.623 },
        { "maxRadius", 1500 },
        { "radiusDecayRate", 0.998 },
        { "isSpiked", false },
        { "isAgitated", false },
        { "canEat", playercells | mothercells | ejected | viruses | food },
        { "avoidSpawningOn", playercells | viruses | mothercells },
        { "speedMultiplier", 1 }
    }},

    { "food", {
        { "baseRadius", 10 },
        { "maxRadius", 20 },
        { "startAmount", 3000 },
        { "maxAmount", 6000 },
        { "canGrow", true },
        { "isSpiked", false },
        { "isAgitated", false },
        { "canEat", nothing },
        { "avoidSpawningOn", nothing }
    }},

    { "virus", {
        { "baseRadius", 100 },
        { "maxRadius", 141.421356237 },
        { "startAmount", 50 },
        { "maxAmount", 50 + 26 },
        { "isSpiked", true },
        { "isAgitated", false },
        { "canEat", ejected },
        { "avoidSpawningOn", playercells | ejected | mothercells },
        { "color", { 0x33, 0xff, 0x33 }}
    }},

    { "motherCell", {
        { "baseRadius", 149 },
        { "maxRadius", 1500 },
        { "startAmount", 25 },
        { "isSpiked", true },
        { "isAgitated", false },
        { "canEat", playercells | ejected | viruses },
        { "avoidSpawningOn", playercells | ejected | mothercells | viruses },
        { "color", { 0xce, 0x63, 0x63 }}
    }},

    { "ejected", {
        { "baseRadius", 36.06 },
        { "maxRadius", 36.06 },
        { "isSpiked", false },
        { "isAgitated", false },
        { "canEat", nothing }
    }}
};
