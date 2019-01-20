#include "Game.hpp"
#include "Map.hpp"
#include "../Player/Player.hpp"
#include "../Modules/Logger.hpp"
#include "../Modules/Commands.hpp"
#include <future>
#include <chrono>
#include <time.h>

using namespace std::chrono;

Game::Game() {
    loadConfig();   // Load config    
    startLogger();  // Start logger
    server.start(); // Start uWS server

    // Wait for server to change running state
    while (server.runningState == -1) {
        Logger::print(".");
    }
    // Server could not start, end game
    if (server.runningState == 0) {
        std::cin.get();
        return;
    }

    map::init(this); // Initialize map
    Commands commands(this); // Command handler
    std::string userInput;

    milliseconds fixed_timestep = (milliseconds)cfg::game_timeStep;
    auto previous = high_resolution_clock::now();
    nanoseconds lag = 0ns;

    while (running) {
        // Get user input without blocking thread
        std::future<std::string&> future = std::async([&]()->std::string& {
            Logger::print("> ");
            std::getline(std::cin, userInput);
            return userInput;
        });
        // Main Loop
        while (future.wait_for(fixed_timestep) == std::future_status::timeout) {
            auto current = high_resolution_clock::now();
            auto elapsed = current - previous;
            previous = current;
            lag += elapsed;

            //processInput();

            while (lag >= fixed_timestep)
                //update();
                lag -= fixed_timestep;

            mainLoop();
        }
        commands.handleUserInput(future.get());
    }
}

void Game::mainLoop() {
    ++tickCount;
    //steady_clock::time_point begin = steady_clock::now();

    // Update players
    for (Player *client : server.clients)
        client->update(tickCount);

    // Update entities
    map::update(tickCount);

    //steady_clock::time_point end = steady_clock::now();
    //Logger::debug("ligma = ", duration_cast<microseconds>(end - begin).count() / 100.0);
}

// Load settings into memory as it is more efficient than
// grabbing them from json object whenever needed
void Game::loadConfig() {
    cfg::logger_maxSeverity = config["logger"]["maxSeverity"];
    cfg::logger_maxFileSeverity = config["logger"]["maxFileSeverity"];
    cfg::logger_printTextColor = static_cast<Logger::Color>((int)config["logger"]["printTextColor"]);
    cfg::logger_backgroundColor = static_cast<Logger::Color>((int)config["logger"]["backgroundColor"]);

    cfg::server_port = config["server"]["port"];
    cfg::server_name = config["server"]["name"].get<std::string>();
    cfg::server_maxConnections = config["server"]["maxConnections"];
    cfg::server_maxSupportedProtocol = config["server"]["maxSupportedProtocol"];
    cfg::server_minSupportedProtocol = config["server"]["minSupportedProtocol"];

    cfg::game_mode = config["game"]["mode"];
    cfg::game_timeStep = config["game"]["timeStep"];
    cfg::game_mapWidth = config["game"]["mapWidth"];
    cfg::game_mapHeight = config["game"]["mapHeight"];
    cfg::game_quadTreeLeafCapacity = config["game"]["quadTreeLeafCapacity"];
    cfg::game_quadTreeMaxDepth = config["game"]["quadTreeMaxDepth"];

    cfg::entity_decelerationPerTick = config["entity"]["decelerationPerTick"];
    cfg::entity_minAcceleration = config["entity"]["minAcceleration"];
    cfg::entity_minEatOverlap = config["entity"]["minEatOverlap"];
    cfg::entity_minEatSizeMult = config["entity"]["minEatSizeMult"];

    cfg::player_maxNameLength = config["player"]["maxNameLength"];
    cfg::player_maxCells = config["player"]["maxCells"];
    cfg::player_maxFreeroamScale = config["player"]["maxFreeroamScale"];
    cfg::player_maxFreeroamSpeed = config["player"]["maxFreeroamSpeed"];
    cfg::player_viewBoxWidth = config["player"]["viewBoxWidth"];
    cfg::player_viewBoxHeight = config["player"]["viewBoxHeight"];
    cfg::player_baseRemergeTime = config["player"]["baseRemergeTime"];
    cfg::player_cellRemoveTime = config["player"]["cellRemoveTime"];
    cfg::player_chanceToSpawnFromEjected = config["player"]["chanceToSpawnFromEjected"];
    cfg::player_collisionIgnoreTime = config["player"]["collisionIgnoreTime"];

    cfg::playerCell_baseRadius = config["playerCell"]["baseRadius"];
    cfg::playerCell_maxRadius = config["playerCell"]["maxRadius"];
    cfg::playerCell_minRadiusToSplit = config["playerCell"]["minRadiusToSplit"];
    cfg::playerCell_minRadiusToEject = config["playerCell"]["minRadiusToEject"];
    cfg::playerCell_minVirusSplitMass = config["playerCell"]["minVirusSplitMass"];
    cfg::playerCell_ejectAngleVariation = config["playerCell"]["ejectAngleVariation"];
    cfg::playerCell_radiusDecayRate = config["playerCell"]["radiusDecayRate"];
    cfg::playerCell_initialAcceleration = config["playerCell"]["initialAcceleration"];
    cfg::playerCell_isSpiked = config["playerCell"]["isSpiked"];
    cfg::playerCell_isAgitated = config["playerCell"]["isAgitated"];
    cfg::playerCell_canEat = config["playerCell"]["canEat"];
    cfg::playerCell_avoidSpawningOn = config["playerCell"]["avoidSpawningOn"];
    cfg::playerCell_speedMultiplier = config["playerCell"]["speedMultiplier"];

    cfg::food_baseRadius = config["food"]["baseRadius"];
    cfg::food_maxRadius = config["food"]["maxRadius"];
    cfg::food_startAmount = config["food"]["startAmount"];
    cfg::food_maxAmount = config["food"]["maxAmount"];
    cfg::food_canGrow = config["food"]["canGrow"];
    cfg::food_isSpiked = config["food"]["isSpiked"];
    cfg::food_isAgitated = config["food"]["isAgitated"];
    cfg::food_canEat = config["food"]["canEat"];
    cfg::food_avoidSpawningOn = config["food"]["avoidSpawningOn"];

    cfg::virus_baseRadius = config["virus"]["baseRadius"];
    cfg::virus_maxRadius = config["virus"]["maxRadius"];
    cfg::virus_startAmount = config["virus"]["startAmount"];
    cfg::virus_maxAmount = config["virus"]["maxAmount"];
    cfg::virus_initialAcceleration = config["virus"]["initialAcceleration"];
    cfg::virus_isSpiked = config["virus"]["isSpiked"];
    cfg::virus_isAgitated = config["virus"]["isAgitated"];
    cfg::virus_canEat = config["virus"]["canEat"];
    cfg::virus_avoidSpawningOn = config["virus"]["avoidSpawningOn"];
    cfg::virus_color = Color(config["virus"]["color"]);

    cfg::motherCell_baseRadius = config["motherCell"]["baseRadius"];
    cfg::motherCell_maxRadius = config["motherCell"]["maxRadius"];
    cfg::motherCell_startAmount = config["motherCell"]["startAmount"];
    cfg::motherCell_maxAmount = config["motherCell"]["maxAmount"];
    cfg::motherCell_isSpiked = config["motherCell"]["isSpiked"];
    cfg::motherCell_isAgitated = config["motherCell"]["isAgitated"];
    cfg::motherCell_canEat = config["motherCell"]["canEat"];
    cfg::motherCell_avoidSpawningOn = config["motherCell"]["avoidSpawningOn"];
    cfg::motherCell_color = Color(config["motherCell"]["color"]);

    cfg::ejected_baseRadius = config["ejected"]["baseRadius"];
    cfg::ejected_maxRadius = config["ejected"]["maxRadius"];
    cfg::ejected_efficiency = config["ejected"]["efficiency"];
    cfg::ejected_initialAcceleration = config["ejected"]["initialAcceleration"];
    cfg::ejected_isSpiked = config["ejected"]["isSpiked"];
    cfg::ejected_isAgitated = config["ejected"]["isAgitated"];
    cfg::ejected_canEat = config["ejected"]["canEat"];
}

void Game::startLogger() {
    Logger::start();
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
    Logger::info("Starting logger...");
    Logger::info("Loading configurations...");
}

Game::~Game() {
    server.end(); // Stop uWS server
    map::clear(); // Clear map

    // End logger
    Logger::warn("Saving log...");
    Logger::end();
}

namespace cfg {

int logger_maxSeverity;
int logger_maxFileSeverity;
Logger::Color logger_printTextColor;
Logger::Color logger_backgroundColor;

short server_port;
std::string server_name;
unsigned long long server_maxConnections;
unsigned int server_maxSupportedProtocol;
unsigned int server_minSupportedProtocol;

unsigned int game_mode;
unsigned int game_timeStep;
double game_mapWidth;
double game_mapHeight;
unsigned int game_quadTreeLeafCapacity;
unsigned int game_quadTreeMaxDepth;

double entity_decelerationPerTick;
double entity_minAcceleration;
double entity_minEatOverlap;
double entity_minEatSizeMult;
double entity_restitutionCoefficient;

unsigned int player_maxNameLength;
unsigned int player_maxCells;
float player_maxFreeroamScale;
double player_maxFreeroamSpeed;
unsigned int player_viewBoxWidth;
unsigned int player_viewBoxHeight;
double player_baseRemergeTime;
unsigned long long player_cellRemoveTime;
int player_chanceToSpawnFromEjected;
unsigned long long player_collisionIgnoreTime;

double playerCell_baseRadius;
double playerCell_maxRadius;
double playerCell_minRadiusToSplit;
double playerCell_minRadiusToEject;
double playerCell_minVirusSplitMass;
double playerCell_ejectAngleVariation;
double playerCell_radiusDecayRate;
double playerCell_initialAcceleration;
bool playerCell_isSpiked;
bool playerCell_isAgitated;
unsigned char playerCell_canEat;
unsigned char playerCell_avoidSpawningOn;
unsigned int playerCell_speedMultiplier;

double food_baseRadius;
double food_maxRadius;
unsigned int food_startAmount;
unsigned int food_maxAmount;
bool food_canGrow;
bool food_isSpiked;
bool food_isAgitated;
unsigned char food_canEat;
unsigned char food_avoidSpawningOn;

double virus_baseRadius;
double virus_maxRadius;
unsigned int virus_startAmount;
unsigned int virus_maxAmount;
double virus_initialAcceleration;
bool virus_isSpiked;
bool virus_isAgitated;
unsigned char virus_canEat;
unsigned char virus_avoidSpawningOn;
Color virus_color;

double motherCell_baseRadius;
double motherCell_maxRadius;
unsigned int motherCell_startAmount;
unsigned int motherCell_maxAmount;
bool motherCell_isSpiked;
bool motherCell_isAgitated;
unsigned char motherCell_canEat;
unsigned char motherCell_avoidSpawningOn;
Color motherCell_color;

double ejected_baseRadius;
double ejected_maxRadius;
double ejected_efficiency;
double ejected_initialAcceleration;
bool ejected_isSpiked;
bool ejected_isAgitated;
unsigned char ejected_canEat;

} // namespace config