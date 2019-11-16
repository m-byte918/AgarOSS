#pragma once
#include "../Modules/Utils.hpp"
#include "../Connection/Server.hpp"
#include "../Modules/Logger.hpp"
#include "../Modules/Commands.hpp"

enum GameState {
    RUNNING,
    PAUSED,
    ENDED
};
struct Commands;

class Game {
    friend struct Commands;
public:
    Game();
    void mainLoop();
    void updateLeaderboard();
    void loadConfig();
    void startLogger();
    ~Game();

    Commands commands{ nullptr };
    unsigned long long tickCount = 0;

    std::vector<Player*> leaders;
private:
    long long updateTime = 0;
    GameState state = GameState::RUNNING;
    Server server;
};

namespace cfg {

extern std::string logger_logName;
extern std::string logger_logFolder;
extern std::string logger_logBackupFolder;
extern int logger_maxSeverity;
extern int logger_maxFileSeverity;
extern Logger::Color logger_printTextColor;
extern Logger::Color logger_backgroundColor;

extern std::string server_host;
extern short server_port;
extern std::string server_name;
extern unsigned int server_playerBots;
extern unsigned int server_minionsPerPlayer;
extern unsigned long long server_maxConnections;
extern unsigned int server_maxSupportedProtocol;
extern unsigned int server_minSupportedProtocol;

extern unsigned int game_mode;
extern unsigned int game_timeStep;
extern unsigned int game_leaderboardLength;
extern double game_mapWidth;
extern double game_mapHeight;
extern unsigned int game_quadTreeLeafCapacity;
extern unsigned int game_quadTreeMaxDepth;

extern float entity_decelerationPerTick;
extern float entity_minAcceleration;
extern float entity_minEatOverlap;
extern float entity_minEatSizeMult;

extern unsigned int player_maxNameLength;
extern std::string player_skinNameTags;
extern unsigned int player_maxCells;
extern float player_maxFreeroamScale;
extern float player_maxFreeroamSpeed;
extern unsigned int player_viewBoxWidth;
extern unsigned int player_viewBoxHeight;
extern unsigned int player_viewBoxHeight;
extern float player_baseRemergeTime;
extern int player_chanceToSpawnFromEjected;
extern unsigned long long player_collisionIgnoreTime;

extern float playerCell_baseRadius;
extern float playerCell_maxMass;
extern float playerCell_minMassToSplit;
extern float playerCell_minRadiusToEject;
extern float playerCell_minVirusSplitMass;
extern float playerCell_ejectAngleVariation;
extern float playerCell_radiusDecayRate;
extern float playerCell_initialAcceleration;
extern bool playerCell_isSpiked;
extern bool playerCell_isAgitated;
extern unsigned char playerCell_canEat;
extern unsigned char playerCell_avoidSpawningOn;
extern unsigned int playerCell_speedMultiplier;

extern float food_baseRadius;
extern float food_maxRadius;
extern unsigned int food_startAmount;
extern unsigned int food_maxAmount;
extern bool food_canGrow;
extern bool food_isSpiked;
extern bool food_isAgitated;
extern unsigned char food_canEat;
extern unsigned char food_avoidSpawningOn;

extern float virus_baseRadius;
extern float virus_maxRadius;
extern unsigned int virus_startAmount;
extern unsigned int virus_maxAmount;
extern float virus_initialAcceleration;
extern bool virus_isSpiked;
extern bool virus_isAgitated;
extern unsigned char virus_canEat;
extern unsigned char virus_avoidSpawningOn;
extern Color virus_color;

extern float motherCell_baseRadius;
extern float motherCell_maxRadius;
extern unsigned int motherCell_startAmount;
extern unsigned int motherCell_maxAmount;
extern bool motherCell_isSpiked;
extern bool motherCell_isAgitated;
extern unsigned char motherCell_canEat;
extern unsigned char motherCell_avoidSpawningOn;
extern Color motherCell_color;

extern float ejected_baseRadius;
extern float ejected_maxRadius;
extern float ejected_efficiency;
extern float ejected_initialAcceleration;
extern bool ejected_isSpiked;
extern bool ejected_isAgitated;
extern unsigned char ejected_canEat;

} // namespace config