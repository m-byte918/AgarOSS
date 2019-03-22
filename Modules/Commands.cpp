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
            if (p->cellName() == arg) player = p;
        for (Minion *m : game->server.minions)
            if (m->cellName() == arg) player = (Player*)m;
        for (PlayerBot *b : game->server.playerBots)
            if (b->cellName() == arg) player = (Player*)b;
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
    } catch (const char *e) {
        Logger::warn(e, '\n');
    } catch (int index) {
        std::string flag = parsedArgs[index].dump();
        auto execute = [&](unsigned int id) {
            parsedArgs[index] = id;
            try {
                (this->*cmd)(parsedArgs);
            } catch (const char *e) {
                Logger::warn(e);
            }
        };
        if (flag.find('p') != std::string::npos)
            for (Player *player : game->server.clients)
                execute(player->id);
        if (flag.find('b') != std::string::npos)
            for (PlayerBot *bot : game->server.playerBots)
                execute(bot->id);
        if (flag.find('m') != std::string::npos)
            for (Minion *minion : game->server.minions)
                execute(minion->id);
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
        names.push_back(player->cellName());
    }
    for (PlayerBot *bot : game->server.playerBots) {
        if (bot->state()      == PlayerState::DEAD)         states.push_back("DEAD");
        else if (bot->state() == PlayerState::PLAYING)      states.push_back("PLAYING");
        else if (bot->state() == PlayerState::DISCONNECTED) states.push_back("DISCONNECTED");

        ids.push_back(std::to_string(bot->id));
        protocols.push_back("?");
        cells.push_back(std::to_string(bot->cells.size()));
        scores.push_back(std::to_string((int)bot->score()));
        centers.push_back(bot->center().toString());
        names.push_back(bot->cellName());
    }
    // sort strings by size
    auto compare = [](const std::string& first, const std::string& second) {
        return first.size() < second.size();
    };
    std::sort(ids.begin(),       ids.end(),       compare);
    std::sort(states.begin(),    states.end(),    compare);
    std::sort(protocols.begin(), protocols.end(), compare);
    std::sort(cells.begin(),     cells.end(),     compare);
    std::sort(scores.begin(),    scores.end(),    compare);
    std::sort(centers.begin(),   centers.end(),   compare);
    std::sort(names.begin(),     names.end(),     compare);
    unsigned int idSize       = (unsigned int)ids.back().size();
    unsigned int stateSize    = (unsigned int)states.back().size();
    unsigned int protocolSize = (unsigned int)protocols.back().size();
    unsigned int cellSize     = (unsigned int)cells.back().size();
    unsigned int scoreSize    = (unsigned int)scores.back().size();
    unsigned int centerSize   = (unsigned int)centers.back().size();
    unsigned int nameSize     = (unsigned int)names.back().size();

    // print columns + whitespace the size of the largest string to keep
    // the columns neat and organized no matter the length of a single row
    std::string ID     = "ID"       + std::string(idSize       < 2 ? 1 : idSize,       ' ');
    std::string STATE  = "STATE"    + std::string(stateSize    < 5 ? 1 : stateSize,    ' ');
    std::string P      = "P"        + std::string(protocolSize < 1 ? 1 : protocolSize, ' ');
    std::string CELLS  = "CELLS"    + std::string(cellSize     < 5 ? 1 : cellSize,     ' ');
    std::string SCORE  = "SCORE"    + std::string(scoreSize    < 5 ? 1 : scoreSize,    ' ');
    std::string CENTER = "POSITION" + std::string(centerSize   < 8 ? 1 : centerSize,   ' ');
    std::string NAME   = "NAME"     + std::string(nameSize     < 4 ? 1 : nameSize,     ' ');
    Logger::print(ID + "| " + STATE + "| " + P + "| " + CELLS + "| " + SCORE + "| " + CENTER + "| " + NAME + "\n");
    Logger::print(std::string(ID.size() + STATE.size() + P.size() + CELLS.size()
        + SCORE.size() + CENTER.size() + NAME.size() + 12, '-') + "\n");

    // print rows
    for (unsigned long long i = 0; i < totalPlayers; ++i) {
        Logger::print(
            " "  + ids[i]       + std::string(ID.size()     - ids[i].size() - 1,       ' ') +
            "| " + states[i]    + std::string(STATE.size()  - states[i].size(),    ' ') +
            "| " + protocols[i] + std::string(P.size()      - protocols[i].size(), ' ') +
            "| " + cells[i]     + std::string(CELLS.size()  - cells[i].size(),     ' ') +
            "| " + scores[i]    + std::string(SCORE.size()  - scores[i].size(),    ' ') +
            "| " + centers[i]   + std::string(CENTER.size() - centers[i].size(),   ' ') +
            "| " + names[i]     + std::string(NAME.size()   - names[i].size(),     ' ') + "\n"
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
        Logger::info("Despawning ", map::entities[CellType::FOOD].size(), " food.");
        while (!map::entities[CellType::FOOD].empty())
            map::despawn(map::entities[CellType::FOOD].back().get());
        cfg::food_startAmount = tempStartAmount;
    }
    if (type == "viruses" || type == "all") {
        tempStartAmount = cfg::virus_startAmount;
        cfg::virus_startAmount = 0;
        Logger::info("Despawning ", map::entities[CellType::VIRUS].size(), " viruses.");
        while (!map::entities[CellType::VIRUS].empty())
            map::despawn(map::entities[CellType::VIRUS].back().get());
        cfg::virus_startAmount = tempStartAmount;
    }
    if (type == "ejected" || type == "all") {
        Logger::info("Despawning ", map::entities[CellType::EJECTED].size(), " ejected.");
        while (!map::entities[CellType::EJECTED].empty())
            map::despawn(map::entities[CellType::EJECTED].back().get());
    }
    if (type == "mothercells" || type == "all") {
        tempStartAmount = cfg::motherCell_startAmount;
        cfg::motherCell_startAmount = 0;
        Logger::info("Despawning ", map::entities[CellType::MOTHERCELL].size(), " mothercells.");
        while (!map::entities[CellType::MOTHERCELL].empty())
            map::despawn(map::entities[CellType::MOTHERCELL].back().get());
        cfg::motherCell_startAmount = tempStartAmount;
    }
    if (type == "playercells" || type == "all") {
        Logger::info("Despawning ", map::entities[CellType::PLAYERCELL].size(), " playercells.");
        while (!map::entities[CellType::PLAYERCELL].empty())
            map::despawn(map::entities[CellType::PLAYERCELL].back().get());
    }
}

void Commands::spawn(const std::vector<json> &args) {
    if (args.size() < 2 || args.size() > 8 || !args[0].is_number_unsigned() || !args[1].is_string())
        throw "Invalid arguments.";
    if (args.size() == 3 || args.size() == 6 || args.size() == 7)
        throw "Invalid argument length.";

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
    float radius = config[type]["baseRadius"];
    if (args.size() >= 5) {
        if (!args[4].is_number_unsigned())
            throw "Mass must be unsigned.";
        radius = std::max(utils::toRadius(args[4]), radius);
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

        if (type == "food") map::spawn<Food>(position, radius, color);
        else if (type == "virus") map::spawn<Virus>(position, radius, color);
        else if (type == "ejected") {
            e_ptr &e = map::spawn<Ejected>(position, radius, color);
            e->setCreator(1);
            e->setVelocity(
                (float)rand(1.0, (double)cfg::ejected_initialAcceleration),
                rand(0.0, 2.0) * (double)MATH_PI
            );
        }
        else map::spawn<MotherCell>(position, radius, color);
    }
    std::string positionStr = positionProvided ? position.toString() : "random position";
    std::string colorStr = colorProvided ? "color of " + color.toString() : "random color";
    Logger::info("Spawned ", args[0].dump(), " " + type + " at " + positionStr + " with ",
        (unsigned)utils::toMass(radius), " mass and a " + colorStr);
}

void Commands::playerbot(const std::vector<json> &args) {
    Logger::warn("Playerbots are not implemented yet");

    /*if (args.size() > 1 || args.size() == 1 && !args[0].is_number_unsigned())
        throw "Invalid arguments.";

    unsigned int amount = args.size() == 1 ? args[0] : 1;

    if (amount == 0) {
        for (PlayerBot *bot : game->server.playerBots)
            bot->onDisconnection();
    } else for (unsigned int i = 0; i < amount; ++i) {
        PlayerBot *bot = new PlayerBot(&game->server);
        bot->setCellName("bot " + std::to_string(bot->id));
        game->server.clients.push_back(bot);
    }
    Logger::print(amount == 0 ? "Removed all" : "Added ", amount, " player bots.\n");*/
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

    for (Entity *cell : player->cells)
        cell->setPosition({ x, y });

    Logger::print("Set position of " + player->cellName() + " to "
        + player->center().toString() + "\n");
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
    if (player->state() != PlayerState::PLAYING)
        throw "Player is not in-game.";

    if (args[1] == 0) {
        Logger::print("Removed ", player->minions.size(), " minions from " + player->cellName());
        while (!player->minions.empty())
            player->minions.back()->onDisconnection();
    } else {
        float spawnRadius = args.size() < 3 ? cfg::playerCell_baseRadius : utils::toRadius(args[2]);
        if (spawnRadius < cfg::playerCell_baseRadius)
            throw "Size cannot be less than cfg::playerCell_baseRadius.";

        std::string name = args.size() < 4 ? player->skinName() + player->cellName() : args[3].get<std::string>();

        for (unsigned int i = args[1]; i > 0; --i) {
            Minion *minion = new Minion(&game->server, player);
            minion->setCellName(name);
            minion->spawnRadius = spawnRadius;
            player->minions.push_back(minion);
        }
        Logger::print("Added " + args[1].dump(), " minions for " + player->cellName(), 
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

    PlayerCell::Entity *biggestCell = player->cells[0];
    for (Entity *cell : player->cells) {
        if (cell->radius() > biggestCell->radius())
            biggestCell = cell;
    }
    biggestCell->pop();
    Logger::print("Popped " + player->cellName() + "\n");
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
    
    Logger::print("Killed " + player->cellName() + "\n");
}

void Commands::merge(const std::vector<json> &args) {
    if (args.size() != 1 || (!args[0].is_number_unsigned() && !args[0].is_string()))
        throw "Invalid arguments.";
    if (args[0].is_string() && checkFlagStr(args[0], "pbm"))
        throw 0;

    Player *player = getPlayer(args[0]);
    player->isForceMerging = !player->isForceMerging;

    Logger::print(player->cellName() + " is ", player->isForceMerging
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

    float radius = utils::toRadius(args[1]);

    if (radius < cfg::playerCell_baseRadius)
        throw "Size cannot be less than cfg::playerCell_baseRadius.";

    for (Entity *cell : player->cells)
        cell->setRadius(radius);

    Logger::print("Set mass of " + player->cellName() + " to " + args[1].dump(), '\n');
}

void Commands::setname(const std::vector<json> &args) {
    if (args.size() != 2 || !args[1].is_string() ||
        (!args[0].is_number_unsigned() && !args[0].is_string()))
        throw "Invalid arguments.";
    if (args[0].is_string() && checkFlagStr(args[0], "pbm"))
        throw 0;
    
    Player *player = getPlayer(args[0]);

    std::string oldName = player->cellName();
    player->setCellName(args[1]);

    Logger::print("Set cell name of " + oldName + " to " + player->cellName());
}

void Commands::setskin(const std::vector<json> &args) {
    if (args.size() != 2 || !args[1].is_string() ||
        (!args[0].is_number_unsigned() && !args[0].is_string()))
        throw "Invalid arguments.";
    if (args[0].is_string() && checkFlagStr(args[0], "pbm"))
        throw 0;

    Player *player = getPlayer(args[0]);
    player->setSkinName(args[1]);

    Logger::print("Set skin name of " + player->cellName() + " to " + player->skinName());
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

    Logger::print("Set spawn mass of " + player->cellName() + " to " + args[1].dump(), '\n');
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

    for (Entity *cell : player->cells) {
        amountOfEjected = (int)std::ceil(cell->mass() / ejectedBaseMass);
        for (; amountOfEjected > 0; --amountOfEjected) {
            e_ptr &e = map::spawn<Ejected>(cell->position(), cfg::ejected_baseRadius, cell->color());
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
    for (Entity *cell : player->cells)
        cell->speedMultiplier = args[1];

    Logger::info("Set speed multiplier of " + player->cellName() + " to " + args[1].dump());
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
    for (Entity *cell : player->cells)
        cell->setColor(newColor);

    Logger::info("Set color of " + player->cellName() + " to " + newColor.toString());
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

    Logger::info("Split " + player->cellName() + " " + args[1].dump() + " times");
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
        Vec2 position = player->cells.back()->position();
        float radius = player->cells.back()->radius();
        Color color = player->cells.back()->color();
        map::despawn(player->cells.back());
        if (type == "mothercell")
            map::spawnUnsafe<MotherCell>(position, radius, color);
        else if (type == "virus")
            map::spawnUnsafe<Virus>(position, radius, color);
        else if (type == "ejected")
            map::spawnUnsafe<Ejected>(position, radius, color);
        else if (type == "food")
            map::spawnUnsafe<Food>(position, radius, color);
    }
    Logger::info("Replaced the cells of " + player->cellName() + " with " + type);
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

    Logger::info("Reloading configurations...\n");
    game->loadConfig(); // Reload config
}

void Commands::debug(const std::vector<json> &args) {
    args;
    Logger::info("Clients: ", game->server.clients.size());
    Logger::info("Minions: ", game->server.minions.size());
    Logger::info("Player Bots: ", game->server.playerBots.size());
    Logger::info();
    Logger::info("Food: ", map::entities[CellType::FOOD].size());
    Logger::info("Viruses: ", map::entities[CellType::VIRUS].size());
    Logger::info("Ejected: ", map::entities[CellType::EJECTED].size());
    Logger::info("MotherCells: ", map::entities[CellType::MOTHERCELL].size());
    Logger::info("PlayerCells: ", map::entities[CellType::PLAYERCELL].size());
    Logger::info("Total quadTree objects: ", map::quadTree.totalObjects());
    Logger::info("Total quadTree children: ", map::quadTree.totalChildren());
    Logger::info();
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
        "\n| spawn, add <amount> <type> [(<x> <y>)] [mass] [(<r> <g> <b>)]                      |"
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
        "\n|                                      Sets mass of a player                         |"
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