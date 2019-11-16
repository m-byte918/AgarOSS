#include "Commands.hpp"
#include "../Game/Game.hpp"
#include "Logger.hpp"
#include "../Game/Map.hpp"
#include "../Player/Player.hpp"
#include "../Player/Minion.hpp"
#include "../Entities/Food.hpp"
#include "../Entities/Virus.hpp"
#include "../Player/PlayerBot.hpp"
#include "../Entities/Ejected.hpp"
#include "../Entities/MotherCell.hpp"
#include "../Entities/PlayerCell.hpp"

Commands::Commands(Game *_game) :
    game(_game) {
}

Player *Commands::getPlayer(const json &arg) {
    Player *player = nullptr;

    if (arg.is_number_unsigned()) {
        for (Player *p : game->server.clients)
            if (p->id == arg) player = p;
        for (Minion *m : game->server.minions)
            if (m->id == arg) player = (Player*)m;
        for (PlayerBot *b : game->server.playerBots)
            if (b->id == arg) player = (Player*)b;
    } else {
        for (Player *p : game->server.clients)
            if (p->cellNameUTF8() == arg) player = p;
        for (Minion *m : game->server.minions)
            if (m->cellNameUTF8() == arg) player = (Player*)m;
        for (PlayerBot *b : game->server.playerBots)
            if (b->cellNameUTF8() == arg) player = (Player*)b;
    }
    if (player == nullptr || player->state() == PlayerState::DISCONNECTED)
        throw "Player does not exist.";
    return player;
}

void Commands::parse(std::string &in) {
    if (in.empty()) return;
    Logger::logMessage(in + '\n'); // Write input to log

    // Split input by whitespace    
    std::vector<std::string> args = splitStr(in, ' ');

    // Convert command name to lowercase
    for (char &c : args[0]) c = (char)::tolower(c);

    // Validate command name
    auto cmd = command[args[0]];
    if (cmd == nullptr) {
        Logger::warn("'", args[0], "'", " is not a valid command.\n");
        return;
    }
    // Validate arguments
    std::vector<json> parsedArgs;
    for (unsigned int i = 1; i < args.size(); ++i) {
        std::string arg = args[i];
        try {
            parsedArgs.push_back(json::parse(arg));
        } catch (json::exception &e) {
            // -pbm flag
            if (checkFlagStr(arg, "pbm")) {
                parsedArgs.push_back(arg);
                continue;
            }
            Logger::warn(e.what(), "\n");
            return;
        }
    }
    // Validation finished, now execute command
    try {
        (this->*cmd)(parsedArgs);
        Logger::print('\n');
    }
    catch (const char *e) {
        Logger::warn(e, '\n');
    }
    catch (int index) {
        std::string flag = parsedArgs[index].dump();
        auto execute = [&](unsigned int id) {
            parsedArgs[index] = id;
            try {
                (this->*cmd)(parsedArgs);
            } catch (const char *e) {
                Logger::warn(e);
            }
        };
        if (flag.find('p') != std::string::npos) {
            for (size_t i = 0; i < game->server.clients.size();) {
                Player *p = game->server.clients[i];
                execute(p->id);
                if (p == game->server.clients[i]) ++i;
            }
        }
        if (flag.find('b') != std::string::npos) {
            for (size_t i = 0; i < game->server.playerBots.size();) {
                PlayerBot *b = game->server.playerBots[i];
                execute(b->id);
                if (b == game->server.playerBots[i]) ++i;
            }
        }
        if (flag.find('m') != std::string::npos) {
            for (size_t i = 0; i < game->server.minions.size();) {
                Minion *m = game->server.minions[i];
                execute(m->id);
                if (m == game->server.minions[i]) ++i;
            }
        }
        Logger::print('\n');
    }
}

// ******************** SERVER COMMANDS ******************** //

void Commands::clr(const std::vector<json> &args) {
    if (!args.empty()) throw "'clr' takes zero arguments.";
    Logger::clearConsole();
}

void Commands::pause(const std::vector<json> &args) {
    if (!args.empty()) throw "'pause' takes zero arguments.";

    if (game->state == GameState::RUNNING) {
        game->state = GameState::PAUSED;
        Logger::warn("Server is now paused.");
    } else if (game->state == GameState::PAUSED) {
        game->state = GameState::RUNNING;
        Logger::warn("Server is no longer paused.");
    }
}

void Commands::playerlist(const std::vector<json> &args) {
    if (!args.empty())
        throw "'playerlist' takes zero arguments.";
    size_t totalPlayers = game->server.clients.size() + game->server.playerBots.size();
    if (totalPlayers == 0)
        throw "No players are connected to the server.";

    // columns
    std::vector<std::string> ids, states, protocols, cells, scores, centers, names;
    int idSize = 2, protocolSize = 1, stateSize = 5, cellSize = 5, scoreSize = 5, centerSize = 8, nameSize = 4;

    // collect information
    for (Player *player : game->server.clients) {
        switch (player->state()) {
        case PlayerState::DEAD:         states.push_back("DEAD");         break;
        case PlayerState::DISCONNECTED: states.push_back("DISCONNECTED"); break;
        case PlayerState::FREEROAM:     states.push_back("FREEROAM");     break;
        case PlayerState::PLAYING:      states.push_back("PLAYING");      break;
        case PlayerState::SPECTATING:   states.push_back("SPECTATING");   break;
        };
        ids.push_back(std::to_string(player->id));
        protocols.push_back(std::to_string(player->protocolNum));
        cells.push_back(std::to_string(player->cells.size()));
        scores.push_back(std::to_string((int)player->score()));
        centers.push_back(player->center().toString());
        names.push_back(player->cellNameUTF8());
        stateSize = std::max(stateSize, (int)states.back().size());
        idSize = std::max(idSize, (int)ids.back().size());
        protocolSize = std::max(protocolSize, (int)protocols.back().size());
        cellSize = std::max(cellSize, (int)cells.back().size());
        scoreSize = std::max(scoreSize, (int)scores.back().size());
        centerSize = std::max(centerSize, (int)centers.back().size());
        nameSize = std::max(nameSize, (int)names.back().size());
    }
    for (PlayerBot *bot : game->server.playerBots) {
        if (bot->state() == PlayerState::DEAD)         states.push_back("DEAD");
        else if (bot->state() == PlayerState::PLAYING)      states.push_back("PLAYING");
        else if (bot->state() == PlayerState::DISCONNECTED) states.push_back("DISCONNECTED");

        ids.push_back(std::to_string(bot->id));
        protocols.push_back("?");
        cells.push_back(std::to_string(bot->cells.size()));
        scores.push_back(std::to_string((int)bot->score()));
        centers.push_back(bot->center().toString());
        names.push_back(bot->cellNameUTF8());
        stateSize = std::max(stateSize, (int)states.back().size());
        idSize = std::max(idSize, (int)ids.back().size());
        protocolSize = std::max(protocolSize, (int)protocols.back().size());
        cellSize = std::max(cellSize, (int)cells.back().size());
        scoreSize = std::max(scoreSize, (int)scores.back().size());
        centerSize = std::max(centerSize, (int)centers.back().size());
        nameSize = std::max(nameSize, (int)names.back().size());
    }
    // print columns + whitespace the size of the largest string to keep
    // the columns neat and organized no matter the length of a single row
    std::string ID = "ID" + std::string(idSize, ' ');
    std::string STATE = "STATE" + std::string(stateSize, ' ');
    std::string P = "P" + std::string(protocolSize, ' ');
    std::string CELLS = "CELLS" + std::string(cellSize, ' ');
    std::string SCORE = "SCORE" + std::string(scoreSize, ' ');
    std::string CENTER = "POSITION" + std::string(centerSize, ' ');
    std::string NAME = "NAME" + std::string(nameSize, ' ');
    Logger::print(ID + "| " + STATE + "| " + P + "| " + CELLS + "| " + SCORE + "| " + CENTER +
        "| " + NAME + "\n");
    Logger::print(std::string(ID.size() + STATE.size() + P.size() + CELLS.size()
        + SCORE.size() + CENTER.size() + NAME.size() + 12, '-') + "\n");

    // print rows
    for (size_t i = 0; i < totalPlayers; ++i) {
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

void Commands::exit(const std::vector<json> &args) {
    if (!args.empty()) throw "'exit' takes zero arguments.";
    Logger::warn("Closing server...");
    game->state = GameState::ENDED;
}

void Commands::despawn(const std::vector<json> &args) {
    if (args.size() > 1 || (args.size() == 1 && !args[0].is_string()))
        throw "Invalid arguments.";

    std::string type = args.size() == 1 ? args[0].get<std::string>() : "all";

    // Convert type to lowercase
    for (char &c : type) c = (char)::tolower(c);

    if (type != "food" && type != "viruses" && type != "ejected" &&
        type != "mothercells" && type != "playercells" && type != "all")
        throw "Invalid type. Valid types are food, viruses, ejected, mothercells, and playercells.";

    // Start amounts must be temporarily reset to prevent hard locc
    unsigned int tempStartAmount = cfg::food_startAmount;

    if (type == "food" || type == "all") {
        cfg::food_startAmount = 0;
        Logger::info("Despawning ", map::entities[Food::TYPE].size(), " food.");
        while (!map::entities[Food::TYPE].empty())
            map::despawn(map::entities[Food::TYPE].back());
        cfg::food_startAmount = tempStartAmount;
    }
    if (type == "viruses" || type == "all") {
        tempStartAmount = cfg::virus_startAmount;
        cfg::virus_startAmount = 0;
        Logger::info("Despawning ", map::entities[Virus::TYPE].size(), " viruses.");
        while (!map::entities[Virus::TYPE].empty())
            map::despawn(map::entities[Virus::TYPE].back());
        cfg::virus_startAmount = tempStartAmount;
    }
    if (type == "ejected" || type == "all") {
        Logger::info("Despawning ", map::entities[Ejected::TYPE].size(), " ejected.");
        while (!map::entities[Ejected::TYPE].empty())
            map::despawn(map::entities[Ejected::TYPE].back());
    }
    if (type == "mothercells" || type == "all") {
        tempStartAmount = cfg::motherCell_startAmount;
        cfg::motherCell_startAmount = 0;
        Logger::info("Despawning ", map::entities[MotherCell::TYPE].size(), " mothercells.");
        while (!map::entities[MotherCell::TYPE].empty())
            map::despawn(map::entities[MotherCell::TYPE].back());
        cfg::motherCell_startAmount = tempStartAmount;
    }
    if (type == "playercells" || type == "all") {
        Logger::info("Despawning ", map::entities[PlayerCell::TYPE].size(), " playercells.");
        while (!map::entities[PlayerCell::TYPE].empty())
            map::despawn(map::entities[PlayerCell::TYPE].back());
    }
}

void Commands::spawn(const std::vector<json> &args) {
    if (args.size() < 2 || args.size() > 8 || !args[0].is_number_unsigned() || !args[1].is_string())
        throw "Invalid arguments.";
    if (args.size() == 4 || args.size() == 6 || args.size() == 7)
        throw "Invalid argument length.";

    // Validate entity type
    std::string type = args[1].get<std::string>();
    if (type != "food" && type != "virus" && type != "ejected" && type != "motherCell")
        throw "Invalid entity type. Valid types are food, virus, ejected, and motherCell.";
    // Validate radius
    float radius = config[type]["baseRadius"];
    if (args.size() >= 3) {
        if (!args[2].is_number_unsigned())
            throw "Mass must be unsigned.";
        radius = std::max(utils::toRadius(args[2]), radius);
    }
    // Validate position
    Vec2 position;
    bool positionProvided = args.size() >= 5;
    if (positionProvided) {
        if (!args[3].is_number() || !args[4].is_number())
            throw "Coordinates must be a number.";
        double x = args[3];
        double y = args[4];
        Rect bound = map::bounds();

        if (x < bound.left() || x > bound.right() || y < bound.bottom() || y > bound.top())
            throw "Position is out of bounds.";

        position = { x, y };
    }
    // Validate color
    Color color;
    bool colorProvided = args.size() > 5 && args.size() < 9;
    if (colorProvided) {
        if (!args[5].is_number_unsigned() || !args[6].is_number_unsigned() ||
            !args[7].is_number_unsigned()) throw "Color values must be unsigned.";
        if ((unsigned)args[5] > 255 || (unsigned)args[6] > 255 || (unsigned)args[7] > 255)
            throw "Color values must range from 0 to 255.";
        color = Color(args[5], args[6], args[7]);
    }
    // Spawn entities with valid attributes
    for (unsigned int i = args[0]; i > 0; --i) {
        if (!positionProvided) position = randomPosition();
        if (!colorProvided) color = randomColor();

        if (type == "food") {
            map::spawn<Food>(position, radius, colorProvided ? color : randomColor());
        }
        else if (type == "virus") {
            map::spawn<Virus>(position, radius, colorProvided ? color : cfg::virus_color);
        }
        else if (type == "ejected") {
            map::spawn<Ejected>(position, radius, colorProvided ? color : randomColor())->setVelocity(
                (float)rand(1.0, (double)cfg::ejected_initialAcceleration),
                rand(0.0, 2.0) * (double)MATH_PI
            );
        } else {
            map::spawn<MotherCell>(position, radius, colorProvided ? color : cfg::motherCell_color);
        }
    }
    std::string positionStr = positionProvided ? position.toString() : "random position";
    Logger::info("Spawned ", args[0].dump(), " " + type + " at " + positionStr + " with ",
        (unsigned)utils::toMass(radius), " mass and a color of " + color.toString());
}

void Commands::playerbot(const std::vector<json> &args) {
    if (args.size() > 1 || (args.size() == 1 && !args[0].is_number_unsigned()))
        throw "Invalid arguments.";

    unsigned int amount = args.size() == 1 ? args[0].get<unsigned int>() : 1;

    if (amount == 0) {
        while (!game->server.playerBots.empty())
            game->server.playerBots.back()->onDisconnection();
        Logger::print("Removed all player bots.\n");
    } else {
        Logger::info("Spawning ", amount, " player bots...\n");
        for (unsigned int i = 0; i < amount; ++i) {
            PlayerBot *bot = new PlayerBot(&game->server);
            game->server.playerBots.push_back(bot);
            bot->setFullName("bot " + std::to_string(game->server.playerBots.size()));
        }
    }
}

// ******************** PLAYER COMMANDS ******************** //

void Commands::setposition(const std::vector<json> &args) {
    if (args.size() != 3 || !args[1].is_number() || !args[2].is_number()
        || (!args[0].is_number_unsigned() && !args[0].is_string()))
        throw "Invalid arguments.";
    if (args[0].is_string() && checkFlagStr(args[0], "pbm"))
        throw 0;

    Player *player = getPlayer(args[0]);
    if (player->state() != PlayerState::PLAYING)
        throw "Player is not in-game.";

    double x = args[1];
    double y = args[2];
    Rect bounds = map::bounds();

    if (x < bounds.left() || x > bounds.right() || y < bounds.bottom() || y > bounds.top())
        throw "Position is out of bounds.";

    for (sptr<PlayerCell::Entity> cell : player->cells)
        cell->setPosition({ x, y });

    Logger::print("Set position of " + player->cellNameUTF8() + " to "
        + Vec2(x, y).toString() + "\n");
}

void Commands::minion(const std::vector<json> &args) {
    if (args.size() < 2 || args.size() > 4 ||
        !args[1].is_number_unsigned() ||
        (!args[0].is_number_unsigned() && !args[0].is_string()) ||
        (args.size() >= 3 && !args[2].is_number_unsigned()) ||
        (args.size() == 4 && !args[3].is_string()))
        throw "Invalid arguments.";
    if (args[0].is_string() && checkFlagStr(args[0], "pbm"))
        throw 0;
    
    Player *player = getPlayer(args[0]);

    if (args[1] == 0) {
        Logger::print("Removed ", player->minions.size(), " minions from " + player->cellNameUTF8());
        while (!player->minions.empty())
            player->minions.back()->onDisconnection();
    } else {
        float spawnRadius = args.size() < 3 ? cfg::playerCell_baseRadius : utils::toRadius(args[2]);
        spawnRadius = std::max(cfg::playerCell_baseRadius, spawnRadius);

        std::string name = "";
        if (args.size() < 4) {
            if (player->skinName() != "")
                name = cfg::player_skinNameTags.front() + player->skinName() + cfg::player_skinNameTags.back();
            name += player->cellNameUTF8();
        } else {
            name = args[3].get<std::string>();
        }
        // Add minions
        for (unsigned int i = args[1]; i > 0; --i) {
            Minion *minion = new Minion(&game->server, player);
            minion->setFullName(name);
            minion->spawnRadius = spawnRadius;
            player->minions.push_back(minion);
        }
        Logger::print("Added " + args[1].dump(), " minions for " + player->cellNameUTF8(),
            " with the name '", name, "' and a spawn mass of ", utils::toMass(spawnRadius), '\n');
    }
}

void Commands::pop(const std::vector<json> &args) {
    if (args.size() != 1 || (!args[0].is_number_unsigned() && !args[0].is_string()))
        throw "Invalid arguments.";
    if (args[0].is_string() && checkFlagStr(args[0], "pbm"))
        throw 0;

    Player *player = getPlayer(args[0]);
    if (player->state() != PlayerState::PLAYING)
        throw "Player is not in-game.";

    sptr<PlayerCell::Entity> biggestCell = player->cells[0];
    for (sptr<PlayerCell::Entity> cell : player->cells) {
        if (cell->radius() > biggestCell->radius())
            biggestCell = cell;
    }
    biggestCell->pop();
    Logger::print("Popped " + player->cellNameUTF8() + "\n");
}

void Commands::kill(const std::vector<json> &args) {
    if (args.size() != 1 || (!args[0].is_number_unsigned() && !args[0].is_string()))
        throw "Invalid arguments.";
    if (args[0].is_string() && checkFlagStr(args[0], "pbm"))
        throw 0;

    Player *player = getPlayer(args[0]);
    if (player->state() != PlayerState::PLAYING)
        throw "Player is not in-game.";

    while (!player->cells.empty())
        map::despawn(player->cells.back());

    Logger::print("Killed " + player->cellNameUTF8() + "\n");
}

void Commands::merge(const std::vector<json> &args) {
    if (args.size() != 1 || (!args[0].is_number_unsigned() && !args[0].is_string()))
        throw "Invalid arguments.";
    if (args[0].is_string() && checkFlagStr(args[0], "pbm"))
        throw 0;

    Player *player = getPlayer(args[0]);
    player->isForceMerging = !player->isForceMerging;

    Logger::print(player->cellNameUTF8() + " is ", player->isForceMerging
        ? "" : "no longer ", "force merging", '\n');
}

void Commands::setmass(const std::vector<json> &args) {
    if (args.size() != 2 || !args[1].is_number_unsigned() ||
        (!args[0].is_number_unsigned() && !args[0].is_string()))
        throw "Invalid arguments.";
    if (args[0].is_string() && checkFlagStr(args[0], "pbm"))
        throw 0;

    Player *player = getPlayer(args[0]);
    if (player->state() != PlayerState::PLAYING)
        throw "Player is not in-game.";
    if (args[1] == 0)
        throw "Mass cannot be zero.";

    float avgMass = args[1].get<float>() / player->cells.size();
    for (sptr<PlayerCell::Entity> cell : player->cells)
        cell->setMass(avgMass);

    Logger::print("Set mass of " + player->cellNameUTF8() + " to " + args[1].dump(), '\n');
}

void Commands::setname(const std::vector<json> &args) {
    if (args.size() != 2 || !args[1].is_string() ||
        (!args[0].is_number_unsigned() && !args[0].is_string()))
        throw "Invalid arguments.";
    if (args[0].is_string() && checkFlagStr(args[0], "pbm"))
        throw 0;

    Player *player = getPlayer(args[0]);

    std::string oldName = player->cellNameUTF8();
    player->setCellName(args[1].get<std::string>());

    Logger::print("Set cell name of " + oldName + " to " + player->cellNameUTF8());
}

void Commands::setskin(const std::vector<json> &args) {
    if (args.size() != 2 || !args[1].is_string() ||
        (!args[0].is_number_unsigned() && !args[0].is_string()))
        throw "Invalid arguments.";
    if (args[0].is_string() && checkFlagStr(args[0], "pbm"))
        throw 0;

    Player *player = getPlayer(args[0]);
    player->setSkinName(args[1]);

    Logger::print("Set skin name of " + player->cellNameUTF8() + " to " + player->skinName());
}

void Commands::spawnmass(const std::vector<json> &args) {
    if (args.size() != 2 || !args[1].is_number_unsigned() ||
        (!args[0].is_number_unsigned() && !args[0].is_string()))
        throw "Invalid arguments.";
    if (args[0].is_string() && checkFlagStr(args[0], "pbm"))
        throw 0;

    Player *player = getPlayer(args[0]);
    float radius = utils::toRadius(args[1]);

    if (radius < cfg::playerCell_baseRadius)
        throw "Size cannot be less than cfg::playerCell_baseRadius.";

    player->spawnRadius = radius;

    Logger::print("Set spawn mass of " + player->cellNameUTF8() + " to " + args[1].dump(), '\n');
}

void Commands::explode(const std::vector<json> &args) {
    if (args.size() != 1 || (!args[0].is_number_unsigned() && !args[0].is_string()))
        throw "Invalid arguments.";
    if (args[0].is_string() && checkFlagStr(args[0], "pbm"))
        throw 0;

    Player *player = getPlayer(args[0]);
    if (player->state() != PlayerState::PLAYING)
        throw "Player is not in-game.";

    float ejectedBaseMass = utils::toMass(cfg::ejected_baseRadius);
    int amountOfEjected;

    for (sptr<PlayerCell::Entity> cell : player->cells) {
        amountOfEjected = (int)std::ceil(cell->mass() / ejectedBaseMass);
        for (; amountOfEjected > 0; --amountOfEjected) {
            sptr<Ejected> e = map::spawn<Ejected>(cell->position(), cfg::ejected_baseRadius, cell->color());
            e->setCreator(cell->nodeId());
            e->setVelocity(
                (float)rand(1.0, (double)cfg::ejected_initialAcceleration * 2),
                rand(0.0, 2.0) * (double)MATH_PI
            );
        }
        cell->setRadius(cfg::playerCell_baseRadius);
    }
}

void Commands::speed(const std::vector<json> &args) {
    if (args.size() != 2 || !args[1].is_number_unsigned() ||
        (!args[0].is_number_unsigned() && !args[0].is_string()))
        throw "Invalid arguments.";
    if (args[0].is_string() && checkFlagStr(args[0], "pbm"))
        throw 0;

    Player *player = getPlayer(args[0]);
    for (sptr<PlayerCell::Entity> cell : player->cells)
        cell->speedMultiplier = args[1];

    Logger::info("Set speed multiplier of " + player->cellNameUTF8() + " to " + args[1].dump());
}

void Commands::color(const std::vector<json> &args) {
    if (args.size() != 4 || (!args[0].is_number_unsigned() && !args[0].is_string())
        || !args[1].is_number_unsigned() || !args[2].is_number_unsigned() ||
        !args[3].is_number_unsigned())
        throw "Invalid arguments.";
    if (args[0].is_string() && checkFlagStr(args[0], "pbm"))
        throw 0;
    if ((unsigned)args[1] > 255 || (unsigned)args[2] > 255 || (unsigned)args[3] > 255)
        throw "Color values must range from 0 to 255.";

    Player *player = getPlayer(args[0]);
    if (player->state() != PlayerState::PLAYING)
        throw "Player is not in-game.";

    Color newColor(args[1], args[2], args[3]);
    for (sptr<PlayerCell::Entity> cell : player->cells)
        cell->setColor(newColor);

    Logger::info("Set color of " + player->cellNameUTF8() + " to " + newColor.toString());
}

void Commands::split(const std::vector<json> &args) {
    if (args.size() != 2 || !args[1].is_number_unsigned() ||
        (!args[0].is_number_unsigned() && !args[0].is_string()))
        throw "Invalid arguments.";
    if (args[0].is_string() && checkFlagStr(args[0], "pbm"))
        throw 0;

    Player *player = getPlayer(args[0]);
    if (player->state() != PlayerState::PLAYING)
        throw "Player is not in-game.";

    for (unsigned int i = args[1]; i > 0; --i)
        player->onSplit();

    Logger::info("Split " + player->cellNameUTF8() + " " + args[1].dump() + " times");
}

void Commands::replace(const std::vector<json> &args) {
    if (args.size() != 2 || !args[1].is_string() ||
        (!args[0].is_number_unsigned() && !args[0].is_string()))
        throw "Invalid arguments.";
    if (args[0].is_string() && checkFlagStr(args[0], "pbm"))
        throw 0;

    std::string type = args[1].get<std::string>();

    // Convert type to lowercase
    for (char &c : type) c = (char)::tolower(c);

    if (type != "food" && type != "virus" && type != "ejected" && type != "mothercell")
        throw "Invalid type. Valid types are food, virus, ejected, and mothercell.";

    Player *player = getPlayer(args[0]);
    if (player->state() != PlayerState::PLAYING)
        throw "Player is not in-game.";

    while (!player->cells.empty()) {
        e_ptr cell = player->cells.back();
        if (type == "mothercell")
            map::spawn<MotherCell>(cell->position(), cell->radius(), cell->color(), false);
        else if (type == "virus")
            map::spawn<Virus>(cell->position(), cell->radius(), cell->color(), false);
        else if (type == "ejected")
            map::spawn<Ejected>(cell->position(), cell->radius(),
                cell->color(), false)->setCreator(cell->nodeId());
        else if (type == "food")
            map::spawn<Food>(cell->position(), cell->radius(), cell->color(), false);
        map::despawn(cell);
    }
    Logger::info("Replaced the cells of " + player->cellNameUTF8() + " with " + type);
}

void Commands::kick(const std::vector<json> &args) {
    if (args.size() != 1 || (!args[0].is_number_unsigned() && !args[0].is_string()))
        throw "Invalid arguments.";
    if (args[0].is_string() && checkFlagStr(args[0], "pbm"))
        throw 0;

    Player *player = getPlayer(args[0]);

    if (player->owner == nullptr) {
        if (player->socket == nullptr)
            ((PlayerBot*)player)->onDisconnection();
        else
            player->socket->close();
    } else {
        ((Minion*)player)->onDisconnection();
    }
    Logger::info("Kicked " + player->cellNameUTF8());
}

void Commands::pstring(const std::vector<json>& args) {
    if (args.size() != 1 || (!args[0].is_number_unsigned() && !args[0].is_string()))
        throw "Invalid arguments.";
    if (args[0].is_string() && checkFlagStr(args[0], "pbm"))
        throw 0;
    Logger::info('\n' + getPlayer(args[0])->toString());
}

// ******************** MISCELLANEOUS ******************** //

void Commands::toradius(const std::vector<json> &args) {
    if (args.size() != 1 || !args[0].is_number_float())
        throw "Invalid arguments.";
    Logger::info(utils::toRadius(args[0]));
}

void Commands::tomass(const std::vector<json> &args) {
    if (args.size() != 1 || !args[0].is_number_float())
        throw "Invalid arguments.";
    Logger::info(utils::toMass(args[0]));
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

void Commands::getconfig(const std::vector<json> &args) {
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
        group = args[1].get<std::string>();
    }
    if (!val.is_null()) Logger::info(val.dump(4));
    else discardCopiedValue(args[0], group);
}

void Commands::setconfig(const std::vector<json> &args) {
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

    game->loadConfig(); // Reload config
}

void Commands::debug(const std::vector<json> &args) {
    // Calculate average score
    double avgScore = 0;
    for (Player *player : game->server.clients) avgScore += player->score();
    for (PlayerBot *bot : game->server.playerBots) avgScore += bot->score();
    size_t botAmount = game->server.playerBots.size();
    size_t clientAmount = game->server.clients.size();
    if (clientAmount + botAmount > 0)
        avgScore /= clientAmount + botAmount;

    // Print debug info
    Logger::info("Clients: ", clientAmount);
    Logger::info("Minions: ", game->server.minions.size());
    Logger::info("Player Bots: ", botAmount);
    Logger::info();
    Logger::info("Average player score: ", avgScore);
    Logger::info();
    Logger::info("Food: ", map::entities[Food::TYPE].size());
    Logger::info("Viruses: ", map::entities[Virus::TYPE].size());
    Logger::info("Ejected: ", map::entities[Ejected::TYPE].size());
    Logger::info("MotherCells: ", map::entities[MotherCell::TYPE].size());
    Logger::info("PlayerCells: ", map::entities[PlayerCell::TYPE].size());
    Logger::info("Total quadTree objects: ", map::quadTree.totalObjects());
    Logger::info("Total quadTree children: ", map::quadTree.totalChildren());
    Logger::info();
    Logger::info("Current game tick: ", game->tickCount);
    Logger::info("Update time for Game::mainLoop(): ", game->updateTime, "ms");
}

void Commands::help(const std::vector<json> &args) {
    if (!args.empty()) throw "'help' takes zero arguments.";

    Logger::info(
        "\nUsage: command [options] <args>...                                                    "
        "\n+----------------------------------Server commands-----------------------------------+"
        "\n|                                                                                    |"
        "\n| clr                                  Clears console output                         |"
        "\n| pause                                Pauses the server                             |"
        "\n| playerlist, pl                       Prints a list of information for each player  |"
        "\n| exit, stop, shutdown                 Stops the game and closes the server          |"
        "\n| despawn, rm [<type>]                 Despawns all entities of the specified type   |"
        "\n| spawn, add <amount> <type> [mass] [(<x> <y>)] [(<r> <g> <b>)]                      |"
        "\n|                                      Spawns an entity.                             |"
        "\n| playerbot, pb [<amount>]             Adds or removes playerbots to the server      |"
        "\n|                                                                                    |"
        "\n+----------------------------------Player commands-----------------------------------+"
        "\n| Options:                                                                           |"
        "\n|   -pbm                               Executes command for all players, player bots,|"
        "\n|                                      or minions                                    |"
        "\n|                                                                                    |"
        "\n| setposition, tp (<playerid>|<playername>) <x> <y>                                  |"
        "\n|                                      Sets position of a player's cells             |"
        "\n| minion, mn (<playerid>|<playername>) [<amount> <mass> <name>]                      |"
        "\n|                                      Adds or removes minions for a player          |"
        "\n| pop (<playerid>|<playername>)        Pops a player's cells                         |"
        "\n| kill (<playerid>|<playername>)       Kills a player                                |"
        "\n| merge, mg (<playerid>|<playername>)  Forces a player's cells to remerge            |"
        "\n| setmass, sm (<playerid>|<playername>) <mass>                                       |"
        "\n|                                      Sets absolute mass of a player                |"
        "\n| setname, sn (<playerid>|<playername>) <name>                                       |"
        "\n|                                      Sets a player's cell name                     |"
        "\n| setskin, ss (<playerid>|<playername>) <name>                                       |"
        "\n|                                      Sets a player's skin name                     |"
        "\n| spawnmass, spm (<playerid|<playername>) <mass>                                     |"
        "\n|                                      Sets the spawnmass of a player                |"
        "\n| explode, ex (<playerid>|<playername>)                                              |"
        "\n|                                      Explodes a player into ejected mass           |"
        "\n| speed (<playerid>|<playername>)      Sets the movement speed of a player's cells   |"
        "\n| color (<playerid>|<playername>) <r> <g> <b>                                        |"
        "\n|                                      Sets the color of a player's cells            |"
        "\n| split (<playerid>|<playername>) <amount>                                           |"
        "\n|                                      Splits a player the specified amount of times |"
        "\n| replace, rp (<playerid>|<playername>) <type>                                       |"
        "\n|                                      Replaces a player's cells with the specified  |"
        "\n|                                      entity type                                   |"
        "\n| kick (<playerid>|<playername>)       Kicks a player from the server                |"
        "\n| pstring (<playerid>|<playername>)    Prints debug string of a player               |"
        "\n|                                                                                    |"
        "\n+-----------------------------------Miscellaneous------------------------------------+"
        "\n|                                                                                    |"
        "\n| toradius, tr <mass>                  Converts provided mass to radius              |"
        "\n| tomass, tm <radius>                  Converts provided radius to mass              |"
        "\n| getconfig, gc [<group> [<name>]]     Gets value of config[group][name]             |"
        "\n| setconfig, sc <group> <name> <value> Sets value of config[group][name]             |"
        "\n| debug                                Prints amount of players, entities, and the   |"
        "\n|                                      update time for Game::mainLoop()              |"
        "\n| help                                 Prints a list of all commands + agruments     |"
        "\n|                                                                                    |"
        "\n+------------------------------------------------------------------------------------+"
    );
}

Commands::~Commands() {
}