#pragma once
#include <uWS/uWS.h>
//#include "Player.h"
#include "Modules/Logger.h"
#include "Entity/EntityHandler.h"

// todo: put implementation of functions in .cpp
// for some reason g++ doesn't like it when i do that

class Game {
private:
    uWS::Hub hub;
    uint64_t serverConnections;
    uint64_t tickCount = 0;
public:
    void onClientConnection() {
        hub.onConnection([&](uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest req) {
            if (++serverConnections > cfg::maxConnections) {
                ws->close(1000, "Server connection limit reached");
                return;
            }
            std::cout << "connection made\n";
            //ws->setUserData(new Player);
            //ws->send();
        });
    }
    void onClientDisconnection() {
        hub.onDisconnection([&](uWS::WebSocket<uWS::SERVER> *ws, int code, char *message, size_t length) {
            //std::cout << code << "\n";
            std::cout << "disconnection\n";
            serverConnections--;
        });
    }
    void onClientMessage() {
        hub.onMessage([](uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode) {
            std::cout << "message recieved\n";

            if (length > 256) {
                ws->close(1009, "Spam");
            }
        });
    }
    void run() {
        tickCount++;
        std::cout << (int)tickCount << "\n";
        onClientConnection();
        onClientDisconnection();
        onClientMessage();
    }
    Game() {
        logger::start();

        std::string port_str = std::to_string(cfg::serverPort);

        if (hub.listen(cfg::serverPort)) {
            hub.connect("ws://localhost:" + port_str, nullptr);
            std::cout << "Server is listening on port " << port_str << "\n";
        } else {
            std::cout << "Server couldn't listen on port " << port_str << "\n";
            return;
        }

        // Spawn starting entities
        ent::spawnStartingEntities();

        Timer *timer = new Timer(hub.getLoop());
        timer->setData((void*) this);

        timer->start([](Timer *timer) {
            //auto *timerData = (Game*)timer->getData();
            //timerData->run();
            ((Game*)timer->getData())->run();
        }, 0, cfg::tickStep);

        hub.run();
    }
    ~Game() {
        logger::end();    // Shutdown logger
        ent::removeAll(); // Remove entities
        hub.getDefaultGroup<uWS::SERVER>().close(); // Close server
    }
};
