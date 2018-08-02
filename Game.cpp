#include <future>
#include <chrono>
#include "Game.hpp"
#include "Player.hpp"
#include "Modules/Logger.hpp"
using namespace std::chrono;

Game::Game() {
    Logger::start(); // Start logger
    server.start();  // Start uWS server
    map.init();      // Initialize map

    Commands commands(this); // Command handler

    // fixed timestep of 1 / (60 fps) = 16 milliseconds
    nanoseconds fixed_timestep = 16ms;
    nanoseconds lag = 0ns;

    std::string userInput;
    auto startTime = steady_clock::now();

    while (running) {
        // Get user input without blocking thread
        std::future<std::string&> future = std::async([&]()->std::string& {
            std::cout << "> ";
            std::getline(std::cin, userInput);
            return userInput;
        });
        // Main Loop
        while (future.wait_for((milliseconds)config["game"]["timeStep"]) == std::future_status::timeout) {
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
    // Update players
    for (uWS::WebSocket<uWS::SERVER> *client : server.clients)
        ((Player*)client->getUserData())->update();

    // Update entities
    map.update(++tickCount);
}

Game::~Game() {
    server.end(); // Stop uWS server
    map.clear();  // Clear map

    // End logger
    Logger::warn("Saving log...");
    Logger::end();
}