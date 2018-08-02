#pragma once
#include <string>
#include <vector>
#include <map>

// Forward declarations
class Game;
class Player;

struct Commands {

    Commands(Game*);

    void handleUserInput(std::string &in);

    // Misc
    Player *getPlayer(const std::string &arg);
    Player *getPlayerById(unsigned long long id);
    Player *getPlayerByName(const std::string &name);

    // Commands
    void help(const std::vector<std::string> &args);
    void exit(const std::vector<std::string> &args);
    void clearmap(const std::vector<std::string> &args);
    void setmass(const std::vector<std::string> &args);
    void setposition(const std::vector<std::string> &args);
    void playerlist(const std::vector<std::string> &args);
    void get(const std::vector<std::string> &args);
    void set(const std::vector<std::string> &args);
    void spawn(const std::vector<std::string> &args);
    void debug(const std::vector<std::string> &args);

    ~Commands();

private:
    std::map<std::string, void(Commands::*)(const std::vector<std::string>&)> command = {
        { "help", &Commands::help },
        { "exit", &Commands::exit },
        { "stop", &Commands::exit },
        { "clearmap", &Commands::clearmap },
        { "setmass", &Commands::setmass },
        { "setposition", &Commands::setposition },
        { "playerlist", &Commands::playerlist },
        { "get", &Commands::get },
        { "set", &Commands::set },
        { "spawn", &Commands::spawn },
        { "debug", &Commands::debug }
    };
    Game *game = nullptr;
};