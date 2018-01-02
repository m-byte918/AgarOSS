#include <algorithm>
#include "GameServer.h"
#include "Player.h"
#include "Entity/Entity.h"

GameServer::GameServer() { 
    logger.start();

    double halfWidth = config<double>("mapWidth") / 2;
    double halfHeight = config<double>("mapHeight") / 2;

    _border = {
        -halfWidth,
        -halfHeight,
        halfWidth,
        halfHeight
    };

    // Start timerLoop
    _timerLoop = new Timer(_hub.getLoop());

    _timerLoop->setData((void*) this);
    _timerLoop->start([](Timer *timer) {
        //auto *timerData = (Game*)timer->getData();
        //timerData->run();
        ((GameServer*)timer->getData())->run();
    }, 0, config<unsigned int>("timeStep"));

    // Start uWS server
    startServer();
}

GameServer::~GameServer() {
    _hub.getDefaultGroup<uWS::SERVER>().close();
    logger.end();
}

void GameServer::run() {
    ++_tickCount;
    updateFood();
    
    for (const auto &client : _clients) {
        Player *player = (Player*)client->getUserData();
        //player->update();
    }
}


void GameServer::updateFood() {
    while (_foods.size() < config<unsigned int>("foodStartAmount")) {
        Food *food = new Food();
        _foods.push_back(food);
    }
}

void GameServer::onClientConnection() {
    _hub.onConnection([&](uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest req) {
        if (ws->getFd() == 7) return; // server

        if (_serverConnections >= config<unsigned int>("maxConnections")) {
            ws->close(1000, "Server connection limit reached");
            return;
        }

        Player *player = new Player(ws);
        player->_owner = this;
        ws->setUserData(player);

        _clients.push_back(ws);

        logger.debug("Connection made");
        ++_serverConnections;
    });
}
void GameServer::onClientDisconnection() {
    _hub.onDisconnection([&](uWS::WebSocket<uWS::SERVER> *ws, int code, char *message, size_t length) {
        Player *player = (Player*)ws->getUserData();
        player->setState(PlayerState::DISCONNECTED);

        delete player;
        _clients.erase(std::remove(_clients.begin(), _clients.end(), ws), _clients.end());

        logger.debug("Disconnection made");
        --_serverConnections;
    });
}

void GameServer::onClientMessage() {
    _hub.onMessage([&](uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode) {
        if (length == 0) return;

        if (length > 256) {
            ws->close(1009, "no spam pls");
            return;
        }

        std::vector<unsigned char> packet(message, message + length);

        Player *player = (Player*)ws->getUserData();
        player->_packet.onPacket(packet);
    });
}
void GameServer::onServerSocketError() {
    _hub.onError([&](int port) {
        logger.error("Error listening on port " + config<unsigned int>("serverPort"));
        logger.error("Close out of applications running on the same port or run this with root priveleges.");
    });
}

void GameServer::startServer() {
    onServerSocketError();
    onClientConnection();
    onClientDisconnection();
    onClientMessage();

    if (_hub.listen(config<unsigned int>("serverPort"))) {
        _hub.connect("ws://localhost:" + config<std::string>("serverPort"), nullptr);
        logger.info("Server is listening on port " + config<std::string>("serverPort"));
    } else {
        logger.error("Server couldn't listen on port " + config<std::string>("serverPort"));
        return;
    }
    _hub.run();
}
