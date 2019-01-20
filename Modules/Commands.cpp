#include "Commands.hpp"
#include "../Game/Game.hpp"
#include "Logger.hpp"
#include "../Game/Map.hpp"
#include "../Player/Player.hpp"
#include "../Entities/Food.hpp"
#include "../Entities/Virus.hpp"
#include "../Entities/Ejected.hpp"
#include "../Entities/MotherCell.hpp"
#include "../Entities/PlayerCell.hpp"

Commands::Commands(Game *_game) :
    game(_game) {
}

Player *Commands::getPlayer(const json &arg) {
    if (arg.is_number_unsigned())
        return getPlayerById(arg);
    else
        return getPlayerByName(arg);
}

Player *Commands::getPlayerById(unsigned long long id) {
    for (Player *player : game->server.clients) {
        if (player->id == id && player->state() != PlayerState::DISCONNECTED) 
            return player;
    }
    return nullptr;
}

Player *Commands::getPlayerByName(const std::string &name) {
    for (Player *player : game->server.clients) {
        if (player->cellName() == name && player->state() != PlayerState::DISCONNECTED) 
            return player;
    }
    return nullptr;
}

void Commands::handleUserInput(std::string &in) {
    if (in.empty()) return;
    Logger::logMessage(in + '\n'); // Write input to log

                                   // Remove all spaces from input
    in.erase(std::remove_if(in.begin(), in.end(), [](char c) {
        return c == ' ';
    }), in.end());

    // Get arguments
    std::vector<std::string> args = splitStr(in, '(');
    std::string name = args[0];

    if (in == name || args.back().back() != ')') {
        Logger::warn("Invalid syntax. Try commandName(args)\n");
        return;
    }
    // Extract arguments between parentheses
    unsigned int s = 0;
    for (const std::string &arg : args) s += (unsigned int)arg.size();
    args = splitStr(std::string(in.begin() + name.size() + 1, in.begin() + s), ',');

    // Parse newly extracted arguments
    std::vector<json> parsedArgs;
    for (const std::string &arg : args) {
        try {
            parsedArgs.push_back(json::parse(arg));
        } catch (std::invalid_argument &e) {
            Logger::warn(std::string(e.what()) + "\n");
            return;
        }
    }
    // Parse commands
    if (auto cmd = command[name]) {
        try {
            (this->*cmd)(parsedArgs);
        } catch (const char *e) {
            Logger::warn(e);
        }
    } else {
        Logger::warn("Invalid command. Use 'help()' for a list of commands.");
    }
    Logger::print('\n');
}

void Commands::help(const std::vector<json> &args) {
    if (!args.empty()) throw "help() takes zero arguments.";

    Logger::info("clr() ------------------------------------------ Clears console output.");
    Logger::info("clearMap() ------------------------------------- Clears the map and removes all entities.");
    Logger::info("playerlist() ----------------------------------- Prints a list of information for each player.");
    Logger::info("exit(), stop() --------------------------------- Stops the game and closes the server.");
    Logger::info("toRadius(mass) ----------------------------------Converts provided mass to radius.");
    Logger::info("toMass(radius) ----------------------------------Converts provided radius to mass.");
    Logger::info("getConfig(name = all) -------------------------- Prints value of config[name].");
    Logger::info("setConfig(name, value) ------------------------- Sets value of config[name].");
    Logger::info("pop(playerid|playername) ----------------------- Pops a player's cells");
    Logger::info("kill(playerid|playername) ---------------------- Kills a player.");
    Logger::info("merge(playerid|playername) --------------------- Forces a player's cells to remerge.");
    Logger::info("getConfig(group, name = all) ------------------- Prints value of config[group][name].");
    Logger::info("setMass(playerid|playername, mass) ------------- Sets mass of a player.");
    Logger::info("setConfig(group, configname, value) ------------ Sets value of config[group][name].");
    Logger::info("setPosition(playerid|playername, x, y) --------- Sets position of a player's cells.");
    Logger::info("spawn(amount, type[, x, y[, mass][, r, g, b]]) - Spawns an entity.");
}

void Commands::exit(const std::vector<json> &args) {
    if (!args.empty()) throw "exit() takes zero arguments.";
    Logger::warn("Closing server...");
    game->running = false;
}

void Commands::kill(const std::vector<json> &args) {
    if (args.size() != 1 || (!args[0].is_number_unsigned() && !args[0].is_string()))
        throw "Invalid arguments.";

    Player *player = getPlayer(args[0]);

    if (player == nullptr)
        throw "Player does not exist.";
    if (player->state() != PlayerState::PLAYING)
        throw "Player is not in-game.";

    while (!player->cells.empty()) {
        map::despawn(player->cells.back());
    }
    Logger::print("Killed " + player->cellName());
}

void Commands::clr(const std::vector<json> &args) {
    if (!args.empty()) throw "exit() takes zero arguments.";
    Logger::clearConsole();
}

void Commands::clearMap(const std::vector<json> &args) {
    if (!args.empty()) throw "clearMap() takes zero arguments.";
    map::clear();
}

void Commands::toMass(const std::vector<json> &args) {
    if (args.size() != 1 || !args[0].is_number_float())
        throw "Invalid arguments.";
    Logger::info(utils::toMass(args[0]));
}

void Commands::toRadius(const std::vector<json> &args) {
    if (args.size() != 1 || !args[0].is_number_float())
        throw "Invalid arguments.";
    Logger::info(utils::toRadius(args[0]));
}

void Commands::pop(const std::vector<json> &args) {
    if (args.size() != 1 || (!args[0].is_number_unsigned() && !args[0].is_string()))
        throw "Invalid arguments.";

    Player *player = getPlayer(args[0]);

    if (player == nullptr)
        throw "Player does not exist.";
    if (player->state() != PlayerState::PLAYING)
        throw "Player is not in-game.";

    PlayerCell::Entity *biggestCell = player->cells[0].get();
    for (e_ptr cell : player->cells) {
        if (cell->radius() > biggestCell->radius())
            biggestCell = cell.get();
    }
    biggestCell->pop();

    Logger::print("Popped " + player->cellName());
}

void Commands::merge(const std::vector<json> &args) {
    if (args.size() != 1 || (!args[0].is_number_unsigned() && !args[0].is_string()))
        throw "Invalid arguments.";

    Player *player = getPlayer(args[0]);

    if (player == nullptr)
        throw "Player does not exist.";
    if (player->state() != PlayerState::PLAYING)
        throw "Player is not in-game.";

    player->isForceMerging = !player->isForceMerging;

    Logger::print(player->cellName() + " is ", player->isForceMerging ? "" : "no longer ", "force merging");
}

void Commands::setMass(const std::vector<json> &args) {
    if (args.size() != 2 ||
        (!args[0].is_number_unsigned() && !args[0].is_string()) ||
        !args[1].is_number_unsigned()) throw "Invalid arguments.";

    Player *player = getPlayer(args[0]);

    if (player == nullptr)
        throw "Player does not exist.";
    if (player->state() != PlayerState::PLAYING)
        throw "Player is not in-game.";

    double radius = utils::toRadius(args[1]);

    if (radius < cfg::playerCell_baseRadius)
        throw "Size cannot be less than playerCell's base radius.";

    for (e_ptr cell : player->cells)
        cell->setMass(args[1]);

    Logger::print("Set mass of " + player->cellName() + " to " + args[1].dump() + "\n");
}

void Commands::setPosition(const std::vector<json> &args) {
    if (args.size() != 3 || (!args[0].is_number_unsigned() && !args[0].is_string())
        || !args[1].is_number() || !args[2].is_number()) throw "Invalid arguments.";

    Player *player = getPlayer(args[0]);

    if (player == nullptr)
        throw "Player does not exist.";
    if (player->state() != PlayerState::PLAYING)
        throw "Player is not in-game.";

    double x = args[1];
    double y = args[2];
    Rect bounds = map::bounds();

    if (x < bounds.left() || x > bounds.right() || y < bounds.bottom() || y > bounds.top())
        throw "Position is out of bounds.";

    for (e_ptr cell : player->cells)
        cell->setPosition({ x, y });
    Logger::print("Set position of " + player->cellName() + " to "
        + player->center().toString() + "\n");
}

void Commands::playerlist(const std::vector<json> &args) {
    if (!args.empty())
        throw "playerlist() takes zero arguments.";
    if (game->server.clients.empty())
        throw "No players are connected to the server.";

    // columns
    std::vector<std::string> ids, states, protocols, cells, scores, centers, names;

    // collect information
    for (Player *player : game->server.clients) {
        ids.push_back(std::to_string(player->id));

        switch (player->state()) {
        case PlayerState::DEAD: states.push_back("DEAD"); break;
        case PlayerState::DISCONNECTED: states.push_back("DISCONNECTED"); break;
        case PlayerState::FREEROAM: states.push_back("FREEROAM"); break;
        case PlayerState::PLAYING: states.push_back("PLAYING"); break;
        case PlayerState::SPECTATING: states.push_back("SPECTATING"); break;
        };
        protocols.push_back(std::to_string(player->protocolNum));
        cells.push_back(std::to_string(player->cells.size()));
        scores.push_back(std::to_string((int)player->score()));
        centers.push_back(player->center().toString());
        names.push_back(player->cellName());
    }
    // sort strings by size
    auto compare = [](const std::string& first, const std::string& second) {
        return first.size() < second.size();
    };
    std::sort(ids.begin(), ids.end(), compare);
    std::sort(states.begin(), states.end(), compare);
    std::sort(protocols.begin(), protocols.end(), compare);
    std::sort(cells.begin(), cells.end(), compare);
    std::sort(scores.begin(), scores.end(), compare);
    std::sort(centers.begin(), centers.end(), compare);
    std::sort(names.begin(), names.end(), compare);
    unsigned int idSize = (unsigned int)ids.back().size();
    unsigned int stateSize = (unsigned int)states.back().size();
    unsigned int protocolSize = (unsigned int)protocols.back().size();
    unsigned int cellSize = (unsigned int)cells.back().size();
    unsigned int scoreSize = (unsigned int)scores.back().size();
    unsigned int centerSize = (unsigned int)centers.back().size();
    unsigned int nameSize = (unsigned int)names.back().size();

    // print columns + whitespace the size of the largest string to keep
    // the columns neat and organized no matter the length of a single row
    std::string ID = "ID" + std::string(idSize       < 2 ? 1 : idSize, ' ');
    std::string STATE = "STATE" + std::string(stateSize    < 5 ? 1 : stateSize, ' ');
    std::string P = "P" + std::string(protocolSize < 1 ? 1 : protocolSize, ' ');
    std::string CELLS = "CELLS" + std::string(cellSize     < 5 ? 1 : cellSize, ' ');
    std::string SCORE = "SCORE" + std::string(scoreSize    < 5 ? 1 : scoreSize, ' ');
    std::string CENTER = "POSITION" + std::string(centerSize   < 8 ? 1 : centerSize, ' ');
    std::string NAME = "NAME" + std::string(nameSize     < 4 ? 1 : nameSize, ' ');

    Logger::print(ID + "| " + STATE + "| " + P + "| " + CELLS + "| " + SCORE + "| " + CENTER + "| " + NAME + "\n");
    Logger::print(std::string(ID.size() + STATE.size() + P.size() + CELLS.size()
        + SCORE.size() + CENTER.size() + NAME.size() + 12, '-') + "\n");

    // print rows
    for (unsigned long long i = 0; i < game->server.clients.size(); ++i) {
        Logger::print(
            " " + ids[i] + std::string(ID.size() - ids[i].size() - 1, ' ') +
            "| " + states[i] + std::string(STATE.size() - states[i].size(), ' ') +
            "| " + protocols[i] + std::string(P.size() - protocols[i].size(), ' ') +
            "| " + cells[i] + std::string(CELLS.size() - cells[i].size(), ' ') +
            "| " + scores[i] + std::string(SCORE.size() - scores[i].size(), ' ') +
            "| " + centers[i] + std::string(CENTER.size() - centers[i].size(), ' ') +
            "| " + names[i] + std::string(NAME.size() - names[i].size(), ' ') + "\n"
        );
    }
}

// TODO: Figure out a better way to do this.
void discardCopiedValue(const std::string &arg1, const std::string &arg2) {
    Logger::warn("Config not found.");

    // Discard value that was copied when accessing element
    if (config[arg1].is_null()) {
        config.erase(arg1);
    } else if (config[arg1].is_object()) {
        config[arg1].erase(arg2);
        if (config[arg1].empty())
            config.erase(arg1);
    }
}

void Commands::getConfig(const std::vector<json> &args) {
    if (args.size() > 2) throw "Invalid arguments.";

    if (args.empty()) {
        Logger::info(config.dump(4));
        return;
    }

    json::value_type val;
    std::string group;

    if (args.size() == 1) {
        if (!args[0].is_string()) throw "Invalid arguments.";
        val = config[args[0].get<std::string>()];
        group = "";
    }
    else {
        if (!args[1].is_string()) throw "Invalid arguments.";
        val = config[args[0].get<std::string>()][args[1].get<std::string>()];
        group = args[1].get<std::string>();
    }
    if (!val.is_null()) Logger::info(val.dump(4));
    else discardCopiedValue(args[0], group);
}

void Commands::setConfig(const std::vector<json> &args) {
    if (args.size() < 2 || args.size() > 3 || !args[0].is_string())
        throw "Invalid arguments.";

    json::value_type found;

    if (args.size() == 2) {
        found = config[args[0].get<std::string>()];
    } else {
        if (!args[1].is_string()) throw "Invalid arguments.";
        found = config[args[0].get<std::string>()][args[1].get<std::string>()];
    }
    if (found.is_null()) {
        discardCopiedValue(args[0], args[1]);
        return;
    }
    if (found.is_object())
        throw "Cannot set value of object.";

    if (args.size() == 2)
        config[args[0].get<std::string>()] = args[1];
    else
        config[args[0].get<std::string>()][args[1].get<std::string>()] = args[2];

    Logger::info("Reloading configurations...");
    game->loadConfig(); // Reload config
}

void Commands::spawn(const std::vector<json> &args) {
    if (args.size() < 2 || args.size() > 8 || !args[0].is_number_unsigned() || !args[1].is_string())
        throw "Invalid arguments.";
    if (args.size() == 3 || args.size() == 6 || args.size() == 7)
        throw "Invalid arguments length.";

    // Validate entity type
    std::string type = args[1].get<std::string>();
    if (type != "food" && type != "virus" && type != "ejected" && type != "motherCell")
        throw "Invalid entity type. Valid types are food, virus, ejected, and motherCell.";

    // Validate position
    Vec2 position;
    bool positionProvided = args.size() >= 4;
    if (positionProvided) {
        if (!args[2].is_number() || !args[3].is_number())
            throw "Coordinates must be a number.";
        double x = args[2];
        double y = args[3];
        Rect bound = map::bounds();

        if (x < bound.left() || x > bound.right() || y < bound.bottom() || y > bound.top())
            throw "Position is out of bounds.";

        position = { x, y };
    }
    // Validate radius
    double radius = config[type]["baseRadius"];
    if (args.size() >= 5) {
        if (!args[4].is_number_unsigned())
            throw "Mass must be an unsigned number.";
        radius = std::max(utils::toRadius(args[4]), radius);
    }
    // Validate color
    Color color;
    bool colorProvided = args.size() > 5 && args.size() < 9;
    if (colorProvided) {
        if (!args[5].is_number_unsigned() || !args[6].is_number_unsigned() ||
            !args[7].is_number_unsigned()) throw "Color values must be an unsigned number.";
        if ((unsigned)args[5] > 255 || (unsigned)args[6] > 255 || (unsigned)args[7] > 255)
            throw "Color values must range from 0 to 255.";
        color = Color(args[5], args[6], args[7]);
    }
    // Spawn entities with valid attributes
    unsigned int amount = args[0];
    for (unsigned int i = 0; i < amount; ++i) {
        if (!positionProvided) position = randomPosition();
        if (!colorProvided) color = randomColor();

        if (type == "food") map::spawn<Food>(position, radius, color);
        else if (type == "virus") map::spawn<Virus>(position, radius, color);
        else if (type == "ejected") {
            e_ptr e = map::spawn<Ejected>(position, radius, color);
            e->setCreator(1);
            e->setVelocity(rand(1.0, cfg::ejected_initialAcceleration), rand(0.0, 2.0) * (double)MATH_PI);
        }
        else map::spawn<MotherCell>(position, radius, color);
    }
    std::string positionStr = positionProvided ? position.toString() : "random position";
    std::string colorStr = colorProvided ? "color of " + color.toString() : "random color";
    Logger::info("Spawned ", amount, " " + type + " at " + positionStr + " with ",
        (unsigned)utils::toMass(radius), " mass and a " + colorStr);
}

void Commands::debug(const std::vector<json> &args) {
    args;
    /*Player *player = getPlayer(args[0]);
    for (Collidable *obj : map::quadTree.getObjectsInBound(player->viewBox)) {
        Entity *entity = std::any_cast<Entity*>(obj->data);
        if (entity->isRemoved)
            Logger::warn("Entity is removed");
        if (entity->needsUpdate)
            Logger::warn("Entity needs an update");
    }
    for (const auto &[nodeId, entity] : player->visibleNodes) {
        if (!entity->isRemoved)
            continue;
        if (entity->needsUpdate)
            Logger::warn("2: Entity needs an update");
    }*/
    Logger::info("Food: ", map::entities[CellType::FOOD].size());
    Logger::info("Viruses: ", map::entities[CellType::VIRUS].size());
    Logger::info("Ejected: ", map::entities[CellType::EJECTED].size());
    Logger::info("MotherCells: ", map::entities[CellType::MOTHERCELL].size());
    Logger::info("PlayerCells: ", map::entities[CellType::PLAYERCELL].size());
    Logger::info("Total quadTree objects: ", map::quadTree.totalObjects());
    Logger::info("Total quadTree children: ", map::quadTree.totalChildren());
}

Commands::~Commands() {
}