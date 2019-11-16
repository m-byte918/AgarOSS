#pragma once
#include <map>
#include "Utils.hpp"

// Forward declarations
class Game;
class Player;

struct Commands {

    Commands(Game*);

    // Non commands
    void parse(std::string &in);
    Player *getPlayer(const json &arg);

    // Server commands
    void clr(const std::vector<json> &args);
    void pause(const std::vector<json> &args);
    void playerlist(const std::vector<json> &args);
    void exit(const std::vector<json> &args);
    void despawn(const std::vector<json> &args);
    void spawn(const std::vector<json> &args);
    void playerbot(const std::vector<json> &args);

    // Player commands
    void setposition(const std::vector<json> &args);
    void minion(const std::vector<json> &args);
    void pop(const std::vector<json> &args);
    void kill(const std::vector<json> &args);
    void merge(const std::vector<json> &args);
    void setmass(const std::vector<json> &args);
    void setname(const std::vector<json> &args);
    void setskin(const std::vector<json> &args);
    void spawnmass(const std::vector<json> &args);
    void explode(const std::vector<json> &args);
    void speed(const std::vector<json> &args);
    void color(const std::vector<json> &args);
    void split(const std::vector<json> &args);
    void replace(const std::vector<json> &args);
    void kick(const std::vector<json> &args);
    void pstring(const std::vector<json>& args);

    // Miscellaneous
    void toradius(const std::vector<json> &args);
    void tomass(const std::vector<json> &args);
    void getconfig(const std::vector<json> &args);
    void setconfig(const std::vector<json> &args);
    void debug(const std::vector<json> &args);
    void help(const std::vector<json> &args);
    
    ~Commands();

private:
    std::map<std::string, void(Commands::*)(const std::vector<json>&)> command = {
        { "clr", &Commands::clr },
        { "pause", &Commands::pause },
        { "playerlist", &Commands::playerlist },
        { "pl", &Commands::playerlist },
        { "exit", &Commands::exit },
        { "stop", &Commands::exit },
        { "shutdown", &Commands::exit },
        { "despawn", &Commands::despawn },
        { "rm", &Commands::despawn },
        { "spawn", &Commands::spawn },
        { "add", &Commands::spawn },
        { "playerbot", &Commands::playerbot },
        { "pb", &Commands::playerbot },
        
        { "setposition", &Commands::setposition },
        { "tp", &Commands::setposition },
        { "minion", &Commands::minion },
        { "mn", &Commands::minion },
        { "pop", &Commands::pop },
        { "kill", &Commands::kill },
        { "merge", &Commands::merge },
        { "mg", &Commands::merge },
        { "setmass", &Commands::setmass },
        { "sm", &Commands::setmass },
        { "setname", &Commands::setname },
        { "sn", &Commands::setname },
        { "setskin", &Commands::setskin },
        { "ss", &Commands::setskin },
        { "spawnmass", &Commands::spawnmass },
        { "spm", &Commands::spawnmass },
        { "explode", &Commands::explode },
        { "ex", &Commands::explode },
        { "speed", &Commands::speed },
        { "color", &Commands::color },
        { "split", &Commands::split },
        { "replace", &Commands::replace },
        { "rp", &Commands::replace },
        { "kick", &Commands::kick },
        { "pstring", &Commands::pstring },
        
        { "toradius", &Commands::toradius },
        { "tr", &Commands::toradius },
        { "tomass", &Commands::tomass },
        { "tm", &Commands::tomass },
        { "getconfig", &Commands::getconfig },
        { "gc", &Commands::getconfig },
        { "setconfig", &Commands::setconfig },
        { "sc", &Commands::setconfig },
        { "debug", &Commands::debug },
        { "help", &Commands::help }
    };
    Game *game = nullptr;
};