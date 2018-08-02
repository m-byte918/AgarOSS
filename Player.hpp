#pragma once

#include "Entities/Entities.hpp"
#include "Packets/PacketHandler.hpp"

enum struct PlayerState {
	DEAD = 0,
	PLAYING,
	FREEROAM,
	SPECTATING,
	DISCONNECTED
};
namespace {
unsigned long long prevPlayerId = 0;
}

class Player {
public:
	const unsigned long long id = ++prevPlayerId;
	unsigned int protocol = 0;
	PacketHandler packetHandler;
	uWS::WebSocket<uWS::SERVER> *socket;
	std::vector<PlayerCell::Entity*> cells;

	Player(uWS::WebSocket<uWS::SERVER> *_socket);

	const std::string &getSkinName() const noexcept;
	const std::string &getCellName() const noexcept;
	void setSkinName(const std::string &name) noexcept;
	void setCellName(const std::string &name) noexcept;

	double getScore() const noexcept;
	Vector2 getMouse() const noexcept;
	const PlayerState &getState() const noexcept;
	const Vector2 &getCenter() const noexcept;

	void update();
	void updateScale();
	void updateCenter();
	void updateViewBox();
	void updateVisibleNodes();

	void onSpawn(std::string &name) noexcept;
	void onSpectate() noexcept;
	void onTarget(const Vector2&) noexcept;
	void onSplit() noexcept;
	void onQKey() noexcept;
	void onEject() noexcept;

	void onDisconnection() noexcept;

	~Player();
private:
	std::string skinName = "";
	std::string cellName = "An unnamed cell";
	
	double score = 0;
	double scale = 0;
	double filteredScale = 1;

	Rect viewBox = Rect(0, 0, 0, 0);
	Vector2 center = { 0, 0 };
	Vector2 mouse = { 0, 0 };

	std::vector<Entity*> visibleNodes;

	PlayerState state = PlayerState::DEAD;
};
