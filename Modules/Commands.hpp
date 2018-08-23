#pragma once
#include <map>
#include "Utils.hpp"

// Forward declarations
class Game;
class Player;

struct Commands {

    Commands(Game*);

    void handleUserInput(std::string &in);

    // Misc
    Player *getPlayer(const json &arg);
    Player *getPlayerById(unsigned long long id);
    Player *getPlayerByName(const std::string &name);

    // Commands
    void help(const std::vector<json> &args);
    void exit(const std::vector<json> &args);
    void clearMap(const std::vector<json> &args);
    void toMass(const std::vector<json> &args);
    void toRadius(const std::vector<json> &args);
    void setMass(const std::vector<json> &args);
    void setPosition(const std::vector<json> &args);
    void playerlist(const std::vector<json> &args);
    void getConfig(const std::vector<json> &args);
    void setConfig(const std::vector<json> &args);
    void spawn(const std::vector<json> &args);
    void debug(const std::vector<json> &args);

    ~Commands();

private:
    std::map<std::string, void(Commands::*)(const std::vector<json>&)> command = {
        { "help", &Commands::help },
        { "exit", &Commands::exit },
        { "stop", &Commands::exit },
        { "clearMap", &Commands::clearMap },
        { "toMass", &Commands::toMass },
        { "toRadius", &Commands::toRadius },
        { "setMass", &Commands::setMass },
        { "setPosition", &Commands::setPosition },
        { "playerlist", &Commands::playerlist },
        { "getConfig", &Commands::getConfig },
        { "setConfig", &Commands::setConfig },
        { "spawn", &Commands::spawn },
        { "debug", &Commands::debug }
    };
    Game *game = nullptr;
};