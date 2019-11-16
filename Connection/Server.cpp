#include "Server.hpp"
#include "../Game/Game.hpp"
#include "../Player/Player.hpp"
#include "../Player/Minion.hpp"
#include "../Player/PlayerBot.hpp"
#include "../Modules/Logger.hpp"

const std::string version =
"    _                  ___  ___ ___        _   _ _ ___ \n"
"   /_\\  __ _ __ _ _ _ / _ \\/ __/ __| __ __/ | | | |_  )\n"
"  / _ \\/ _` / _` | '_| (_) \\__ \\__ \\ \\ V /| |_|_  _/ / \n"
" /_/ \\_\\__, \\__,_|_|  \\___/|___/___/  \\_/ |_(_) |_/___|\n"
"       |___/                                           \n\n";

void Server::start() {
    Logger::print(version);
    Logger::info("Starting uWS Server...");
    connectionThread = std::thread([&]() {
        // Because setUserData() no longer exists...x
        struct PerSocketData {
            Player *player = nullptr;
        };
        // Because designated initializers are no longer supported...
        uWS::App::WebSocketBehavior behavior;
        behavior.open = [&](auto *ws, auto *req) {
            if (++connections >= cfg::server_maxConnections) {
                ws->end(1000, "Server connection limit reached");
                return;
            }
            PerSocketData *data = (PerSocketData*)ws->getUserData();
            data->player = new Player(this);
            data->player->socket = ws;
            data->player->packetHandler = PacketHandler(data->player);
            Logger::debug("Connection made");
        };
        behavior.message = [&](auto *ws, std::string_view message, uWS::OpCode opCode) {
            size_t length = message.size();
            if (length == 0) return;
            if (length > 256) {
                ws->end(1009, "no spam pls");
                return;
            }
            std::vector<unsigned char> packet(message.begin(), message.begin() + length);
            Player *player = ((PerSocketData*)ws->getUserData())->player;
            if (player == nullptr)
                return;
            player->packetHandler.onPacket(packet);
            if (player->protocol == nullptr)
                ws->end(1002, "Unsupported protocol");
        };
        behavior.close = [&](auto *ws, int code, std::string_view message) {
            Player* player = ((PerSocketData*)ws->getUserData())->player;
            player->onDisconnection();
            --connections;
            Logger::debug("Disconnection made");
        };
        uWS::App().ws<PerSocketData>("/*", std::move(behavior)).listen(cfg::server_host, cfg::server_port, [&](auto *token) {
            if (token) {
                runningState = 1;
                Logger::print("\n");
                Logger::info("Thread ", std::this_thread::get_id(), " listening on ", cfg::server_host, ":", cfg::server_port);
            } else {
                runningState = 0;
                Logger::error("Thread ", std::this_thread::get_id(), " failed to listen on ", cfg::server_host, ":", cfg::server_port);
                Logger::error("Close out of applications running on the same port or re-run with root priveleges.");
                Logger::print("Press any key to exit...\n");
            }
            conVar.notify_one();
        }).run();
    });
}
void Server::end() {
    Logger::warn("Stopping uWS Server...");
    while (!clients.empty()) {
        Player *player = clients.back();
        player->socket->close();
        delete player;
    }
    while (!minions.empty()) {
        Minion* minion = minions.back();
        minion->onDisconnection();
        delete minion;
    }
    while (!playerBots.empty()) {
        PlayerBot* playerBot = playerBots.back();
        playerBot->onDisconnection();
        delete playerBot;
    }
    connectionThread.detach();
}