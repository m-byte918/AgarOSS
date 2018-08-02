#include "Server.hpp"
#include "Player.hpp"
#include "Modules/Logger.hpp"

void Server::start() {
	Logger::info("Starting uWS Server...");
	connectionThread = std::thread([&]() {
		uWS::Hub hub;
		onClientConnection(&hub);
		onClientDisconnection(&hub);
		onClientMessage(&hub);

		int port = config["server"]["port"];

		if (hub.listen(port)) {
			Logger::info("Server is listening on port " + std::to_string(port));
			hub.run();
		} else {
			Logger::error("Server couldn't listen on port " + std::to_string(port));
			Logger::error("Close out of applications running on the same port or run this with root priveleges.");
			end();
		}
	});
}
void Server::onClientConnection(uWS::Hub *hub) {
	hub->onConnection([&](uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest req) {
		clients.push_back(ws);
		if (++connections >= (unsigned long long)config["server"]["maxConnections"]) {
			ws->close(1000, "Server connection limit reached");
			return;
		}
		// allocating player because it needs to exist outside of this loop
		ws->setUserData(new Player(ws));

		Logger::debug("Connection made");
	});
}
void Server::onClientDisconnection(uWS::Hub *hub) {
	hub->onDisconnection([&](uWS::WebSocket<uWS::SERVER> *ws, int code, char *message, size_t length) {
		((Player*)ws->getUserData())->onDisconnection();
		clients.erase(std::remove(clients.begin(), clients.end(), ws), clients.end());
		ws->setUserData(nullptr);
		--connections;
		Logger::debug("Disconnection made");
	});
}
void Server::onClientMessage(uWS::Hub *hub) {
	hub->onMessage([&](uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode) {
		if (length == 0) return;

		if (length > 256) {
			ws->close(1009, "no spam pls");
			return;
		}
		std::vector<unsigned char> packet(message, message + length);
		Player *player = (Player*)ws->getUserData();
		if (player != nullptr)
			player->packetHandler.onPacket(packet);
	});
}
void Server::end() {
	Logger::warn("Stopping uWS Server...");
	for (uWS::WebSocket<uWS::SERVER> *client : clients)
		client->close();
	connectionThread.detach();
}