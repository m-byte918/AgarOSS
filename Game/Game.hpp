#pragma once
#include "../Modules/Utils.hpp"
#include "../Connection/Server.hpp"
#include "../Modules/Logger.hpp"

struct Commands;
class Game {
    friend struct Commands;
public:
    Game();
    void mainLoop();
    void loadConfig();
    void startLogger();
    ~Game();

    unsigned long long tickCount = 0;
private:
    Server server;
    bool running = true;
};

namespace cfg {

extern int logger_maxSeverity;
extern int logger_maxFileSeverity;
extern Logger::Color logger_printTextColor;
extern Logger::Color logger_backgroundColor;

extern short server_port;
extern std::string server_name;
extern unsigned long long server_maxConnections;
extern unsigned int server_maxSupportedProtocol;
extern unsigned int server_minSupportedProtocol;

extern unsigned int game_mode;
extern unsigned int game_timeStep;
extern double game_mapWidth;
extern double game_mapHeight;
extern unsigned int game_quadTreeLeafCapacity;
extern unsigned int game_quadTreeMaxDepth;

extern double entity_decelerationPerTick;
extern double entity_minAcceleration;
extern double entity_minEatOverlap;
extern double entity_minEatSizeMult;
extern double entity_restitutionCoefficient;

extern unsigned int player_maxNameLength;
extern unsigned int player_maxCells;
extern float player_maxFreeroamScale;
extern double player_maxFreeroamSpeed;
extern unsigned int player_viewBoxWidth;
extern unsigned int player_viewBoxHeight;
extern unsigned int player_viewBoxHeight;
extern double player_baseRemergeTime;
extern unsigned long long player_cellRemoveTime;
extern int player_chanceToSpawnFromEjected;
extern unsigned long long player_collisionIgnoreTime;

extern double playerCell_baseRadius;
extern double playerCell_maxRadius;
extern double playerCell_minRadiusToSplit;
extern double playerCell_minRadiusToEject;
extern double playerCell_minVirusSplitMass;
extern double playerCell_ejectAngleVariation;
extern double playerCell_radiusDecayRate;
extern double playerCell_initialAcceleration;
extern bool playerCell_isSpiked;
extern bool playerCell_isAgitated;
extern unsigned char playerCell_canEat;
extern unsigned char playerCell_avoidSpawningOn;
extern unsigned int playerCell_speedMultiplier;

extern double food_baseRadius;
extern double food_maxRadius;
extern unsigned int food_startAmount;
extern unsigned int food_maxAmount;
extern bool food_canGrow;
extern bool food_isSpiked;
extern bool food_isAgitated;
extern unsigned char food_canEat;
extern unsigned char food_avoidSpawningOn;

extern double virus_baseRadius;
extern double virus_maxRadius;
extern unsigned int virus_startAmount;
extern unsigned int virus_maxAmount;
extern double virus_initialAcceleration;
extern bool virus_isSpiked;
extern bool virus_isAgitated;
extern unsigned char virus_canEat;
extern unsigned char virus_avoidSpawningOn;
extern Color virus_color;

extern double motherCell_baseRadius;
extern double motherCell_maxRadius;
extern unsigned int motherCell_startAmount;
extern unsigned int motherCell_maxAmount;
extern bool motherCell_isSpiked;
extern bool motherCell_isAgitated;
extern unsigned char motherCell_canEat;
extern unsigned char motherCell_avoidSpawningOn;
extern Color motherCell_color;

extern double ejected_baseRadius;
extern double ejected_maxRadius;
extern double ejected_efficiency;
extern double ejected_initialAcceleration;
extern bool ejected_isSpiked;
extern bool ejected_isAgitated;
extern unsigned char ejected_canEat;

} // namespace config