#include "Game.hpp"
#include "Map.hpp"
#include "../Player.hpp"
#include "../Modules/Logger.hpp"
#include "../Modules/Commands.hpp"
#include <future>
#include <chrono>
#include <time.h>

using namespace std::chrono;

Game::Game() {
    // Start logger
    Logger::start();
    Logger::PRINT.suffix = "";

    loadConfig();   // Load config
    server.start(); // Start uWS server
    map::init();    // Initialize map

    Commands commands(this); // Command handler

                             // fixed timestep of 1 / (60 fps) = 16 milliseconds
    nanoseconds fixed_timestep = 16ms;
    nanoseconds lag = 0ns;

    std::string userInput;
    auto startTime = steady_clock::now();

    while (running) {
        // Get user input without blocking thread
        std::future<std::string&> future = std::async([&]()->std::string& {
            Logger::print("> ");
            std::getline(std::cin, userInput);
            return userInput;
        });
        // Main Loop
        while (future.wait_for((milliseconds)cfg::game_timeStep) == std::future_status::timeout) {
            nanoseconds deltaTime = steady_clock::now() - startTime;
            startTime = steady_clock::now();

            lag += deltaTime;

            while (lag >= fixed_timestep)
                lag -= fixed_timestep;

            mainLoop();
        }
        commands.handleUserInput(future.get());
    }
}

void Game::mainLoop() {
    //steady_clock::time_point begin = steady_clock::now();

    // Update players
    for (uWS::WebSocket<uWS::SERVER> *client : server.clients)
        ((Player*)client->getUserData())->update();

    // Update entities
    map::update(++tickCount);

    //steady_clock::time_point end = steady_clock::now();
    //Logger::debug("ligma = ", duration_cast<microseconds>(end - begin).count() / 100.0);
}

// Load settings into memory as it is more efficient than
// grabbing them from json object whenever needed
void Game::loadConfig() {
    Logger::info("Loading configurations...");

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
    cfg::player_minViewBoxScale = config["player"]["minViewBoxScale"];
    cfg::player_viewBoxWidth = config["player"]["viewBoxWidth"];
    cfg::player_viewBoxHeight = config["player"]["viewBoxHeight"];

    cfg::playerCell_baseRadius = config["playerCell"]["baseRadius"];
    cfg::playerCell_maxRadius = config["playerCell"]["maxRadius"];
    cfg::playerCell_minRadiusToSplit = config["playerCell"]["minRadiusToSplit"];
    cfg::playerCell_minRadiusToEject = config["playerCell"]["minRadiusToEject"];
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

Game::~Game() {
    server.end(); // Stop uWS server
    map::clear(); // Clear map

                  // End logger
    Logger::warn("Saving log...");
    Logger::end();
}

namespace cfg {

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

unsigned int player_maxNameLength;
unsigned int player_maxCells;
double player_minViewBoxScale;
unsigned int player_viewBoxWidth;
unsigned int player_viewBoxHeight;

double playerCell_baseRadius;
double playerCell_maxRadius;
double playerCell_minRadiusToSplit;
double playerCell_minRadiusToEject;
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