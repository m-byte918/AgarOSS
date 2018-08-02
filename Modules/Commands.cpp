#include "../Game.hpp"
#include "Commands.hpp"
#include "../Player.hpp"
#include "Logger.hpp"
#include <locale> // tolower

Commands::Commands(Game *_game):
    game(_game) {
}

Player *Commands::getPlayer(const std::string &arg) {
    if (isull(arg))
        return getPlayerById(std::stoull(arg));
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
    Logger::logMessage(in); // Write input to log

    // Get arguments
    std::vector<std::string> input = splitStr(in);
    if (input.empty()) return;

    // Convert first argument to lowercase
    std::string first = input[0];
    std::transform(first.begin(), first.end(), first.begin(), ::tolower);
    
    // Parse commands
    if (auto cmd = command[first])
        (this->*cmd)(input);
    else
        Logger::warn("Invalid command. Use 'help' for a list of commands.");

    std::cout << '\n';
}

void Commands::help(const std::vector<std::string> &args) {
    if (args.size() > 1 && args[1] == "help") return;

    for (const auto &c : command) {
        if (auto cmd = command[c.first])
            (this->*cmd)({ c.first, "help" });
    }
}

void Commands::exit(const std::vector<std::string> &args) {
    if (args.size() > 1 && args[1] == "help") {
        Logger::info(args[0] + " -- Stops the game and closes the server.");
        return;
    }
    Logger::warn("Closing server...");
    game->running = false;
}

void Commands::clearmap(const std::vector<std::string> &args) {
    if (args.size() > 1 && args[1] == "help") {
        Logger::info(args[0] + " -- Clears the map and removes all entities.");
        return;
    }
    game->map.clear();
}

void Commands::setmass(const std::vector<std::string> &args) {
    if (args.size() > 1 && args[1] == "help") {
        Logger::info(args[0] + " [playerid/playername] [mass] -- Sets mass of a player.");
        return;
    }
    if (args.size() != 3 || !isull(args[2])) {
        Logger::warn("Invalid arguments. Use 'setmass help' for more information.");
        return;
    }
    Player *player = getPlayer(args[1]);

    if (player == nullptr) {
        Logger::warn("Player does not exist");
        return;
    } else {
        double size = toSize(std::stod(args[2]));
        if (size < (double)config["playerCell"]["minSize"]) {
            Logger::warn("Size cannot be less than playerCellMinSize");
            return;
        }
        for (Entity *cell : player->cells)
            cell->setSize(size);
    }
    Logger::print("Set mass of " + player->getCellName() + " to " + args[2]);
}

void Commands::setposition(const std::vector<std::string> &args) {
    if (args.size() > 1 && args[1] == "help") {
        Logger::info(args[0] + " [playerid/playername] [X] [Y] -- Sets position of a player's cells.");
        return;
    }
    if (args.size() != 4 || !isull(args[2]) || !isull(args[3])) {
        Logger::warn("Invalid arguments. Use 'setposition help' for more information.");
        return;
    }
    Player *player = getPlayer(args[1]);

    if (player == nullptr) {
        Logger::warn("Player does not exist");
        return;
    } else {
        double x = std::stod(args[2]);
        double y = std::stod(args[3]);
        Rect bounds = Map::getBounds();

        if (x < bounds.left() || x > bounds.right() || y < bounds.bottom() || y > bounds.top()) {
            Logger::warn("Position is out of bounds.");
            return;
        }
        for (Entity *cell : player->cells)
            cell->setPosition({ x, y });
    }
    Logger::print("Set position of " + player->getCellName() + " to " + player->getCenter());
}

void Commands::playerlist(const std::vector<std::string> &args) {
    if (args.size() > 1 && args[1] == "help") {
        Logger::info(args[0] + " -- Prints a list of information for each player.");
        return;
    }
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

void Commands::get(const std::vector<std::string> &args) {
    if (args.size() > 1 && args[1] == "help") {
        Logger::info(args[0] + " [configname/all]     -- Prints value of the specified config."
                "\n              [group] [configname] -- Prints value of the specified config.");
        return;
    }
    if (args.size() < 2 || args.size() > 3) {
        Logger::warn("Invalid arguments. Use 'get help' for more information.");
        return;
    }
    if (args[1] == "all") {
        Logger::info(config.dump(4));
        return;
    }
    auto val = args.size() == 2 ? config[args[1]] : config[args[1]][args[2]];

    if (!val.is_null())
        Logger::info(val.dump(4));
    else
        discardCopiedValue(args[1], args.size() == 2 ? "" : args[2]);
}

void Commands::set(const std::vector<std::string> &args) {
    if (args.size() > 1 && args[1] == "help") {
        Logger::info(args[0] + " [configname] [value]         -- Sets value of the specified config."
                "\n              [group] [configname] [value] -- Sets value of the specified config.");
        return;
    }
    if (args.size() < 3 || args.size() > 4) {
        Logger::warn("Invalid arguments. Use 'set help' for more information.");
        return;
    }
    auto val = args.size() == 3 ? config[args[1]] : config[args[1]][args[2]];
    if (val.is_null()) {
        discardCopiedValue(args[1], args[2]);
        return;
    }
    if (val.is_object()) {
        Logger::warn("Cannot set value of object.");
        return;
    }
    try {
        if (args.size() == 3)
            config[args[1]] = config.parse(args[2]);
        else
            config[args[1]][args[2]] = config.parse(args[3]);
    } catch (...) {
        Logger::warn("Invalid type.");
    }
}

void Commands::spawn(const std::vector<std::string> &args) {
    if (args.size() > 1 && args[1] == "help") {
        Logger::info(args[0] + " [food/virus/ejected/motherCell] [X][Y] [mass] -- Spawns an entity.");
        return;
    }
    if (args.size() < 2 || args.size() > 5 || args.size() == 3) {
        invalidArgs: 
        Logger::warn("Invalid arguments. Use 'spawn help' for more information.");
        return;
    }
    Entity *entity;
    if (args[1] == "food") entity = Map::spawnFood();
    else if (args[1] == "virus") entity = Map::spawnVirus();
    else if (args[1] == "ejected") entity = Map::spawnEjected();
    else if (args[1] == "motherCell") entity = Map::spawnMotherCell();
    else goto invalidArgs;

    if (args.size() > 2) {
        nlohmann::json x, y, mass;
        try {
            x = nlohmann::json::parse(args[2]);
            y = nlohmann::json::parse(args[3]);
            if (args.size() == 5)
                mass = nlohmann::json::parse(args[4]);
        } catch (...) {
            goto invalidArgs;
        }
        Vector2 position;
        if (!x.is_number() || !y.is_number()) {
            Logger::warn("Invalid coordinates, using default instead.");
            position = entity->getPosition();
        } else {
            position = { x, y };
        }
        Rect bound = Map::getBounds();

        if (position.x < bound.left() || position.x > bound.right() ||
            position.y < bound.bottom() || position.y > bound.top()) {
            Logger::warn("Position is out of bounds, using default instead.");
            position = entity->getPosition();
        }
        entity->setPosition(position);

        if (args.size() == 5) {
            if (mass.is_number()) {
                double size = toSize(mass);
                size = std::max(size, config[args[1]]["startSize"].get<double>());
                entity->setSize(size);
            } else {
                Logger::warn("Invalid mass, using default instead.");
            }
        }
    }
    Logger::info("Spawned " + args[1] + " at " + entity->getPosition() 
        + " with a mass of " + std::to_string((unsigned)entity->getMass()));
}

void Commands::debug(const std::vector<std::string> &args) {
    if (args.size() > 1 && args[1] == "help") {
        return;
    }

    Logger::print(2);
}

Commands::~Commands() {
}