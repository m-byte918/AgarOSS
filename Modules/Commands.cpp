#include "Commands.hpp"
#include "../Game.hpp"
#include "Logger.hpp"
#include "../Player.hpp"
#include "../Entities/Map.hpp"

Commands::Commands(Game *_game):
    game(_game) {
}

Player *Commands::getPlayer(const json &arg) {
    if (arg.is_number_unsigned())
        return getPlayerById(arg);
    else
        return getPlayerByName(arg);
}

Player *Commands::getPlayerById(unsigned long long id) {
    for (const auto &socket : game->server.clients) {
        Player *player = (Player*)socket->getUserData();
        if (player->id == id) return player;
    }
    return nullptr;
}

Player *Commands::getPlayerByName(const std::string &name) {
    for (const auto &socket : game->server.clients) {
        Player *player = (Player*)socket->getUserData();
        if (player->getCellName() == name) return player;
    }
    return nullptr;
}

void Commands::handleUserInput(std::string &in) {
    if (in.empty()) return;
    Logger::logMessage(in); // Write input to log

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
    for (const std::string &arg : args) s += arg.size();
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
    std::cout << '\n';
}

void Commands::help(const std::vector<json> &args) {
    if (!args.empty()) throw "help() takes zero arguments.";

    Logger::info("exit() --------------------------------- Stops the game and closes the server.");
    Logger::info("clearMap() ----------------------------- Clears the map and removes all entities.");
    Logger::info("playerlist() --------------------------- Prints a list of information for each player.");
    Logger::info("toRadius(mass) --------------------------Converts provided mass to radius.");
    Logger::info("toMass(radius) --------------------------Converts provided radius to mass.");
    Logger::info("getConfig(name = all) ------------------ Prints value of config[name].");
    Logger::info("setConfig(name, value) ----------------- Sets value of config[name].");
    Logger::info("getConfig(group, name = all) ----------- Prints value of config[group][name].");
    Logger::info("setMass(playerid/playername, mass) ----- Sets mass of a player.");
    Logger::info("setConfig(group, configname, value) ---- Sets value of config[group][name].");
    Logger::info("spawn(type[, x, y[, mass][, r, g, b]]) - Spawns an entity.");
    Logger::info("setPosition(playerid/playername, x, y) - Sets position of a player's cells.");
}

void Commands::exit(const std::vector<json> &args) {
    if (!args.empty()) throw "exit() takes zero arguments.";
    Logger::warn("Closing server...");
    game->running = false;
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

void Commands::setMass(const std::vector<json> &args) {
    if (args.size() != 2 || 
        (!args[0].is_number_unsigned() && !args[0].is_string()) || 
        !args[1].is_number_unsigned()) throw "Invalid arguments.";

    Player *player = getPlayer(args[0]);

    if (player == nullptr)
        throw "Player does not exist.";
    if (player->getState() != PlayerState::PLAYING)
        throw "Player is not in-game.";

    double radius = utils::toRadius(args[1]);

    if (radius < (double)config["playerCell"]["baseRadius"])
        throw "Size cannot be less than playerCell's base radius.";

    for (Entity *cell : player->cells)
        cell->setRadius(radius);
    Logger::print("Set mass of " + player->getCellName() + " to " + args[1].dump());
}

void Commands::setPosition(const std::vector<json> &args) {
    if (args.size() != 3 || (!args[0].is_number_unsigned() && !args[0].is_string())
       || !args[1].is_number() || !args[2].is_number()) throw "Invalid arguments.";

    Player *player = getPlayer(args[0]);

    if (player == nullptr)
        throw "Player does not exist.";
    if (player->getState() != PlayerState::PLAYING)
        throw "Player is not in-game.";

    double x = args[1];
    double y = args[2];
    Rect bounds = map::getBounds();

    if (x < bounds.left() || x > bounds.right() || y < bounds.bottom() || y > bounds.top())
        throw "Position is out of bounds.";

    for (Entity *cell : player->cells)
        cell->setPosition({ x, y });
    Logger::print("Set position of " + player->getCellName() + " to " + player->getCenter());
}

void Commands::playerlist(const std::vector<json> &args) {
    if (!args.empty()) throw "playerlist() takes zero arguments.";

    if (game->server.clients.empty()) {
        Logger::warn("No players are connected to the server.");
        return;
    }
    // columns
    std::vector<std::string> ids, states, protocols, cells, scores, positions, names;

    // collect information
    for (uWS::WebSocket<uWS::SERVER> *socket : game->server.clients) {
        Player *player = (Player*)socket->getUserData();
        ids.push_back(std::to_string(player->id));

        switch (player->getState()) {
            case PlayerState::DEAD:
                states.push_back("DEAD");
                break;
            case PlayerState::DISCONNECTED:
                states.push_back("DISCONNECTED");
                break;
            case PlayerState::FREEROAM:
                states.push_back("FREEROAM");
                break;
            case PlayerState::PLAYING:
                states.push_back("PLAYING");
                break;
            case PlayerState::SPECTATING:
                states.push_back("SPECTATING");
                break;
        };
        protocols.push_back(std::to_string(player->protocol));
        cells.push_back(std::to_string(player->cells.size()));
        scores.push_back(std::to_string(player->getScore()));
        positions.push_back("" + player->getCenter());
        names.push_back(player->getCellName());
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
    std::sort(positions.begin(), positions.end(), compare);
    std::sort(names.begin(), names.end(), compare);

    // print columns + whitespace the size of the largest string to keep
    // the columns neat and organized no matter the length of a single row
    std::string col1 = "ID" + std::string(ids.back().size(), ' ');
    std::string col2 = "STATE" + std::string(states.back().size(), ' ');
    std::string col3 = "P" + std::string(protocols.back().size(), ' ');
    std::string col4 = "CELLS" + std::string(cells.back().size(), ' ');
    std::string col5 = "SCORE" + std::string(scores.back().size(), ' ');
    std::string col6 = "POSITION" + std::string(positions.back().size(), ' ');
    std::string col7 = "NAME" + std::string(names.back().size(), ' ');

    Logger::print(col1 + "| " + col2 + "| " + col3 + "| " + col4 + "| " + col5 + "| " + col6 + "| " + col7);

    Logger::print(std::string(col1.size() + col2.size() + col3.size() 
        + col4.size() + col5.size() + col6.size() + col7.size() + 12, '-'));

    // print rows
    for (unsigned long long i = 0; i < game->server.clients.size(); ++i) {
        std::string id = ids[i] + std::string(ids.back().size() + 2 - ids[i].size(), ' ');
        std::string state = states[i] + std::string(states.back().size() + 7 - states[i].size() - 1, ' ');
        std::string protocol = protocols[i] + std::string(protocols.back().size() + 3 - protocols[i].size() - 1, ' ');
        std::string cellCount = cells[i] + std::string(cells.back().size() + 7 - cells[i].size() - 1, ' ');
        std::string score = scores[i] + std::string(scores.back().size() + 7 - scores[i].size() - 1, ' ');
        std::string position = positions[i] + std::string(positions.back().size() + 10 - positions[i].size() - 1, ' ');
        std::string name = names[i] + std::string(names.back().size() + 6 - names[i].size() - 1, ' ');

        Logger::print(id + "|" + state + "|" + protocol + "|" + cellCount + "|" + score + "|" + position + "|" + name);
    }
}

// Inifficient. Figure out a better way to do this.
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
    } else {
        if (!args[1].is_string()) throw "Invalid arguments.";
        val = config[args[0].get<std::string>()][args[1].get<std::string>()];
        group = args[1];
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
    if (found.is_object()) {
        Logger::warn("Cannot set value of object.");
        return;
    }
    if (args.size() == 2)
        config[args[0].get<std::string>()] = args[1];
    else
        config[args[0].get<std::string>()][args[1].get<std::string>()] = args[2];
}

void Commands::spawn(const std::vector<json> &args) {
    if (args.size() < 1 || args.size() > 7 || !args[0].is_string())
        throw "Invalid arguments.";
    if (args.size() == 2 || args.size() == 5 || args.size() == 6)
        throw "Invalid arguments length.";

    // Validate entity type
    std::string type = args[0].get<std::string>();
    if (type != "food" && type != "virus" && type != "ejected" && type != "motherCell")
        throw "Invalid entity type. Valid types are food, virus, ejected, and motherCell.";

    // Validate position
    Vector2 position = randomPosition();
    if (args.size() >= 3) {
        if (!args[1].is_number() || !args[2].is_number())
            throw "Coordinates must be a number.";
        double x = args[1];
        double y = args[2];
        Rect bound = map::getBounds();

        if (x < bound.left() || x > bound.right() || y < bound.bottom() || y > bound.top())
            throw "Position is out of bounds.";

        position = { x, y };
    }
    // Validate radius
    double radius = config[type]["baseRadius"];
    if (args.size() >= 4) {
        if (!args[3].is_number_unsigned())
            throw "Mass must be an unsigned number.";
        radius = std::max(utils::toRadius(args[3]), radius);
    }
    // Validate color
    Color color = type == "virus" || type == "motherCell" ? config[type]["color"] : randomColor();
    if (args.size() > 4 && args.size() < 8) {
        if (!args[4].is_number_unsigned() || !args[5].is_number_unsigned() ||
            !args[6].is_number_unsigned()) throw "Color values must be an unsigned number.";
        if ((unsigned)args[4] > 255 || (unsigned)args[5] > 255 || (unsigned)args[6] > 255)
            throw "Color values must range from 0 to 255.";
        color = { args[4], args[5], args[6] };
    }
    // Spawn entity with valid attributes
    Entity *entity;
    if (args[0] == "food") entity = map::spawn<Food>(position, radius, color);
    else if (args[0] == "virus") entity = map::spawn<Virus>(position, radius, color);
    else if (args[0] == "ejected") entity = map::spawn<Ejected>(position, radius, color);
    else entity = map::spawn<MotherCell>(position, radius, color);

    Color c = entity->getColor();
    Logger::info("Spawned " + type + " at " + entity->getPosition() + 
        " with a mass of " + std::to_string((unsigned)entity->getMass()) + 
        " and a color of { " + std::to_string(c.r) + ", " + 
        std::to_string(c.g) + ", " + std::to_string(c.b) + " }");
}

void Commands::debug(const std::vector<json> &args) {
    double r1 = args[0];
    double r2 = args[1];
    Logger::print(r1 + (r2 * -0.4));
    Logger::print(r1 - 0.4 * r2);
}

Commands::~Commands() {
}