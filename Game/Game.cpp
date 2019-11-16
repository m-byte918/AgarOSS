#include "Game.hpp"
#include "Map.hpp"
#include "../Player/Player.hpp"
#include "../Player/Minion.hpp"
#include "../Player/PlayerBot.hpp"
#include "../Modules/Logger.hpp"
#include <future>
#include <chrono>
#include <time.h>

using namespace std::chrono;
json config;
std::condition_variable conVar;

Game::Game() {
    Logger::info("Parsing configurations...");
    #ifdef _MSC_VER
        // Why microsoft??
        config = json::parse(std::ifstream("./Settings.json"));
    #else
        config = json::parse(std::ifstream("../Settings.json"));
    #endif
    loadConfig();   // Load config    
    startLogger();  // Start logger
    server.start(); // Start uWS server

    // Wait for server to change running state
    std::mutex myootecks;
    std::unique_lock<std::mutex> locc(myootecks);
    conVar.wait(locc, [&] { return server.runningState != -1; });
    locc.unlock();

    // Server could not start, end game
    if (server.runningState == 0) {
        std::cin.get();
        return;
    }
    commands = Commands(this); // Command handler
    map::init(this);           // Initialize map

    std::string userInput;
    std::future<std::string&> future;

    nanoseconds  lag = 0ns;
    milliseconds timeStep = (milliseconds)cfg::game_timeStep;
    auto         previous = high_resolution_clock::now();
    auto         current = previous;
    auto         elapsed = current - previous;

    while (state != GameState::ENDED) {
        // Get user input without blocking thread
        future = std::async([&]()->std::string& {
            Logger::print("> ");
            std::getline(std::cin, userInput);
            return userInput;
        });
        // Main Loop
        while (future.wait_for(timeStep) == std::future_status::timeout) {
            current = high_resolution_clock::now();
            elapsed = current - previous;
            previous = current;
            lag += elapsed;

            while (lag >= timeStep)
                lag -= timeStep;

            mainLoop();
        }
        commands.parse(future.get());
    }
}
steady_clock::time_point start;
size_t i;
//size_t size;

void Game::mainLoop() {
    if (state == GameState::PAUSED)
        return;

    ++tickCount;
    start = steady_clock::now();

    // Update entities
    map::update();

    // Update non-entities
    for (i = 0; i < server.clients.size(); ++i)
        server.clients[i]->update();
    for (i = 0; i < server.minions.size(); ++i)
        server.minions[i]->update();
    for (i = 0; i < server.playerBots.size(); ++i)
        server.playerBots[i]->update();

    // Update leaderboard once per second
    if (server.clients.size() && tickCount % 25 == 0)
        updateLeaderboard();

    updateTime = duration_cast<milliseconds>(steady_clock::now() - start).count();
}

void Game::updateLeaderboard() {
    leaders.clear();
    leaders.reserve(server.clients.size() + server.playerBots.size());
    // Get eligible leaders
    for (Player *p : server.clients) {
        if (p->state() == PlayerState::PLAYING)
            leaders.push_back(p);
    }
    for (PlayerBot *p : server.playerBots) {
        if (p->state() == PlayerState::PLAYING)
            leaders.push_back(p);
    }
    // Sort and trim leaders
    if (leaders.empty()) return;
    std::sort(leaders.begin(), leaders.end(), [](Player *a, Player *b) { 
        return a->score() > b->score(); 
    });
    if (cfg::game_leaderboardLength < leaders.size())
        leaders.resize(cfg::game_leaderboardLength);
}

// Load settings into memory as it is more efficient than
// grabbing them from json object whenever needed
void Game::loadConfig() {
    Logger::info("Loading configurations into memory...");

    cfg::logger_logName = config["logger"]["logName"].get<std::string>();
    cfg::logger_logFolder = config["logger"]["logFolder"].get<std::string>();
    cfg::logger_logBackupFolder = config["logger"]["logBackupFolder"].get<std::string>();
    cfg::logger_maxSeverity = config["logger"]["maxSeverity"];
    cfg::logger_maxFileSeverity = config["logger"]["maxFileSeverity"];
    cfg::logger_printTextColor = static_cast<Logger::Color>((int)config["logger"]["printTextColor"]);
    cfg::logger_backgroundColor = static_cast<Logger::Color>((int)config["logger"]["backgroundColor"]);

    cfg::server_host = config["server"]["host"].get<std::string>();
    cfg::server_port = config["server"]["port"];
    cfg::server_name = config["server"]["name"].get<std::string>();
    cfg::server_playerBots = config["server"]["playerBots"];
    cfg::server_minionsPerPlayer = config["server"]["minionsPerPlayer"];
    cfg::server_maxConnections = config["server"]["maxConnections"];
    cfg::server_maxSupportedProtocol = config["server"]["maxSupportedProtocol"];
    cfg::server_minSupportedProtocol = config["server"]["minSupportedProtocol"];

    cfg::game_mode = config["game"]["mode"];
    cfg::game_timeStep = config["game"]["timeStep"];
    cfg::game_leaderboardLength = config["game"]["leaderboardLength"];
    cfg::game_mapWidth = config["game"]["mapWidth"];
    cfg::game_mapHeight = config["game"]["mapHeight"];
    cfg::game_quadTreeLeafCapacity = config["game"]["quadTreeLeafCapacity"];
    cfg::game_quadTreeMaxDepth = config["game"]["quadTreeMaxDepth"];

    cfg::entity_decelerationPerTick = config["entity"]["decelerationPerTick"];
    cfg::entity_minAcceleration = config["entity"]["minAcceleration"];
    cfg::entity_minEatOverlap = config["entity"]["minEatOverlap"];
    cfg::entity_minEatSizeMult = config["entity"]["minEatSizeMult"];

    cfg::player_maxNameLength = config["player"]["maxNameLength"];
    cfg::player_skinNameTags = config["player"]["skinNameTags"].get<std::string>();
    cfg::player_maxCells = config["player"]["maxCells"];
    cfg::player_maxFreeroamScale = config["player"]["maxFreeroamScale"];
    cfg::player_maxFreeroamSpeed = config["player"]["maxFreeroamSpeed"];
    cfg::player_viewBoxWidth = config["player"]["viewBoxWidth"];
    cfg::player_viewBoxHeight = config["player"]["viewBoxHeight"];
    cfg::player_baseRemergeTime = config["player"]["baseRemergeTime"];
    cfg::player_chanceToSpawnFromEjected = config["player"]["chanceToSpawnFromEjected"];
    cfg::player_collisionIgnoreTime = config["player"]["collisionIgnoreTime"];

    cfg::playerCell_baseRadius = config["playerCell"]["baseRadius"];
    cfg::playerCell_maxMass = config["playerCell"]["maxMass"];
    cfg::playerCell_minMassToSplit = config["playerCell"]["minMassToSplit"];
    cfg::playerCell_minRadiusToEject = config["playerCell"]["minRadiusToEject"];
    cfg::playerCell_minVirusSplitMass = config["playerCell"]["minVirusSplitMass"];
    cfg::playerCell_ejectAngleVariation = config["playerCell"]["ejectAngleVariation"];
    cfg::playerCell_radiusDecayRate = config["playerCell"]["radiusDecayRate"];
    cfg::playerCell_initialAcceleration = config["playerCell"]["initialAcceleration"];
    cfg::playerCell_isSpiked = config["playerCell"]["isSpiked"];
    cfg::playerCell_isAgitated = config["playerCell"]["isAgitated"];
    cfg::playerCell_canEat = getFlagFrom(config["playerCell"]["canEat"]);
    cfg::playerCell_avoidSpawningOn = getFlagFrom(config["playerCell"]["avoidSpawningOn"]);
    cfg::playerCell_speedMultiplier = config["playerCell"]["speedMultiplier"];

    cfg::food_baseRadius = config["food"]["baseRadius"];
    cfg::food_maxRadius = config["food"]["maxRadius"];
    cfg::food_startAmount = config["food"]["startAmount"];
    cfg::food_maxAmount = config["food"]["maxAmount"];
    cfg::food_canGrow = config["food"]["canGrow"];
    cfg::food_isSpiked = config["food"]["isSpiked"];
    cfg::food_isAgitated = config["food"]["isAgitated"];
    cfg::food_canEat = getFlagFrom(config["food"]["canEat"]);
    cfg::food_avoidSpawningOn = getFlagFrom(config["food"]["avoidSpawningOn"]);

    cfg::virus_baseRadius = config["virus"]["baseRadius"];
    cfg::virus_maxRadius = config["virus"]["maxRadius"];
    cfg::virus_startAmount = config["virus"]["startAmount"];
    cfg::virus_maxAmount = config["virus"]["maxAmount"];
    cfg::virus_initialAcceleration = config["virus"]["initialAcceleration"];
    cfg::virus_isSpiked = config["virus"]["isSpiked"];
    cfg::virus_isAgitated = config["virus"]["isAgitated"];
    cfg::virus_canEat = getFlagFrom(config["virus"]["canEat"]);
    cfg::virus_avoidSpawningOn = getFlagFrom(config["virus"]["avoidSpawningOn"]);
    cfg::virus_color = Color(config["virus"]["color"]);

    cfg::motherCell_baseRadius = config["motherCell"]["baseRadius"];
    cfg::motherCell_maxRadius = config["motherCell"]["maxRadius"];
    cfg::motherCell_startAmount = config["motherCell"]["startAmount"];
    cfg::motherCell_maxAmount = config["motherCell"]["maxAmount"];
    cfg::motherCell_isSpiked = config["motherCell"]["isSpiked"];
    cfg::motherCell_isAgitated = config["motherCell"]["isAgitated"];
    cfg::motherCell_canEat = getFlagFrom(config["motherCell"]["canEat"]);
    cfg::motherCell_avoidSpawningOn = getFlagFrom(config["motherCell"]["avoidSpawningOn"]);
    cfg::motherCell_color = Color(config["motherCell"]["color"]);

    cfg::ejected_baseRadius = config["ejected"]["baseRadius"];
    cfg::ejected_maxRadius = config["ejected"]["maxRadius"];
    cfg::ejected_efficiency = config["ejected"]["efficiency"];
    cfg::ejected_initialAcceleration = config["ejected"]["initialAcceleration"];
    cfg::ejected_isSpiked = config["ejected"]["isSpiked"];
    cfg::ejected_isAgitated = config["ejected"]["isAgitated"];
    cfg::ejected_canEat = getFlagFrom(config["ejected"]["canEat"]);
}

void Game::startLogger() {
    Logger::info("Starting logger...");
    #ifdef _MSC_VER
        // Why??
        Logger::start(cfg::logger_logName, cfg::logger_logFolder, cfg::logger_logBackupFolder);
    #else
        Logger::start(cfg::logger_logName, "."+cfg::logger_logFolder, "."+cfg::logger_logBackupFolder);
    #endif
    Logger::PRINT.suffix = "";
    Logger::setSeverity(cfg::logger_maxSeverity);
    Logger::setFileSeverity(cfg::logger_maxFileSeverity);
    Logger::setConsoleColor(cfg::logger_backgroundColor);
    Logger::setTextColor(cfg::logger_printTextColor);
    Logger::ERR.bgColor = cfg::logger_backgroundColor;
    Logger::INFO.bgColor = cfg::logger_backgroundColor;
    Logger::WARN.bgColor = cfg::logger_backgroundColor;
    Logger::PRINT.bgColor = cfg::logger_backgroundColor;
    Logger::FATAL.bgColor = cfg::logger_backgroundColor;
    Logger::DEBUG.bgColor = cfg::logger_backgroundColor;
}

Game::~Game() {
    map::cleanup(); // Clear map
    server.end();   // Stop uWS server

    // End logger
    Logger::warn("Saving log...");
    Logger::end();
}

namespace cfg {

std::string logger_logName;
std::string logger_logFolder;
std::string logger_logBackupFolder;
int logger_maxSeverity;
int logger_maxFileSeverity;
Logger::Color logger_printTextColor;
Logger::Color logger_backgroundColor;

std::string server_host;
short server_port;
std::string server_name;
unsigned int server_playerBots;
unsigned int server_minionsPerPlayer;
unsigned long long server_maxConnections;
unsigned int server_maxSupportedProtocol;
unsigned int server_minSupportedProtocol;

unsigned int game_mode;
unsigned int game_timeStep;
unsigned int game_leaderboardLength;
double game_mapWidth;
double game_mapHeight;
unsigned int game_quadTreeLeafCapacity;
unsigned int game_quadTreeMaxDepth;

float entity_decelerationPerTick;
float entity_minAcceleration;
float entity_minEatOverlap;
float entity_minEatSizeMult;

unsigned int player_maxNameLength;
std::string player_skinNameTags;
unsigned int player_maxCells;
float player_maxFreeroamScale;
float player_maxFreeroamSpeed;
unsigned int player_viewBoxWidth;
unsigned int player_viewBoxHeight;
float player_baseRemergeTime;
int player_chanceToSpawnFromEjected;
unsigned long long player_collisionIgnoreTime;

float playerCell_baseRadius;
float playerCell_maxMass;
float playerCell_minMassToSplit;
float playerCell_minRadiusToEject;
float playerCell_minVirusSplitMass;
float playerCell_ejectAngleVariation;
float playerCell_radiusDecayRate;
float playerCell_initialAcceleration;
bool playerCell_isSpiked;
bool playerCell_isAgitated;
unsigned char playerCell_canEat;
unsigned char playerCell_avoidSpawningOn;
unsigned int playerCell_speedMultiplier;

float food_baseRadius;
float food_maxRadius;
unsigned int food_startAmount;
unsigned int food_maxAmount;
bool food_canGrow;
bool food_isSpiked;
bool food_isAgitated;
unsigned char food_canEat;
unsigned char food_avoidSpawningOn;

float virus_baseRadius;
float virus_maxRadius;
unsigned int virus_startAmount;
unsigned int virus_maxAmount;
float virus_initialAcceleration;
bool virus_isSpiked;
bool virus_isAgitated;
unsigned char virus_canEat;
unsigned char virus_avoidSpawningOn;
Color virus_color;

float motherCell_baseRadius;
float motherCell_maxRadius;
unsigned int motherCell_startAmount;
unsigned int motherCell_maxAmount;
bool motherCell_isSpiked;
bool motherCell_isAgitated;
unsigned char motherCell_canEat;
unsigned char motherCell_avoidSpawningOn;
Color motherCell_color;

float ejected_baseRadius;
float ejected_maxRadius;
float ejected_efficiency;
float ejected_initialAcceleration;
bool ejected_isSpiked;
bool ejected_isAgitated;
unsigned char ejected_canEat;

} // namespace config