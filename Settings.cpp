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

    // to get size from mass, use toSize(mass)

    { "playerCell", {
        { "startSize", 32 },
        { "minSize", 32 },
        { "decayRate", 0.002 },
        { "isSpiked", false },
        { "isAgitated", false }
    }},

    { "food", {
        { "startSize", 10 },
        { "startAmount", 3000 },
        { "isSpiked", false },
        { "isAgitated", false }
    }},

    { "virus", {
        { "startSize", 100 },
        { "startAmount", 50 },
        { "isSpiked", true },
        { "isAgitated", false },
        { "color", { 0x33, 0xff, 0x33 }}
    }},

    { "motherCell", {
        { "startSize", 149 },
        { "startAmount", 25 },
        { "isSpiked", true },
        { "isAgitated", false },
        { "color", { 0xce, 0x63, 0x63 }}
    }},

    { "ejected", {
        { "startSize", 36.06 },
        { "isSpiked", false },
        { "isAgitated", false }
    }}
};