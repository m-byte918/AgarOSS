#include <algorithm>
#include "GameServer.h"
#include "Player.h"

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
	
    for (const auto &client : _clients) {
        Player *player = (Player*)client->getUserData();
        player->update();
    }
}

void GameServer::onClientConnection() {
    _hub.onConnection([&](uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest req) {
    if (ws->getFd() == 7) return;

        Player *player = new Player(ws);
        ws->setUserData(player);

        _clients.push_back(ws);

        logger.debug("Connection made");
        ++_serverConnections;
    });
}
void GameServer::onClientDisconnection() {
    _hub.onDisconnection([&](uWS::WebSocket<uWS::SERVER> *ws, int code, char *message, size_t length) {
        delete (Player*)ws->getUserData();
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
        }

        std::string msg(message, length);
        std::vector<unsigned char> packet(msg.begin(), msg.end());

        Player *userData = (Player*)ws->getUserData();
        userData->_packetHandler.recievePacket(packet);
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
