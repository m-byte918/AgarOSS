#pragma once
#include "../Connection/Server.hpp"
#include "../Protocol/Protocol.hpp"
#include "../Entities/PlayerCell.hpp"

enum struct PlayerState {
    DEAD = 0,
    PLAYING,
    FREEROAM,
    SPECTATING,
    DISCONNECTED
};
namespace {
unsigned int prevPlayerId = 0;
}

class Player {
public:
    // Server, protocol
    Server       *server;
    Protocol     *protocol;
    PacketHandler packetHandler;
    uWS::WebSocket<uWS::SERVER> *socket;

    // Misc
    unsigned int       id;
    unsigned int       protocolNum       = 0;
    unsigned long long disconnectionTick = 0;
    bool               isForceMerging    = false;
    std::vector<std::shared_ptr<PlayerCell::Entity>> cells;

    // Setters
    Player(uWS::WebSocket<uWS::SERVER> *_socket);
    void setDead() noexcept;
    void setFreeroam() noexcept;
    void setSpectating() noexcept;
    void setSkinName(const std::string &name) noexcept;
    void setCellName(const std::string &name) noexcept;
    
    // Getters
    double score() const noexcept;
    const Vec2 &mouse() const noexcept;
    const Vec2 &center() const noexcept;
    const PlayerState &state() const noexcept;
    const std::string &skinName() const noexcept;
    const std::string &cellName() const noexcept;

    // Updating
    void update(unsigned long long tick);
    void updateScale();
    void updateCenter();
    void updateViewBox();
    void updateVisibleNodes();

    // Recieved information
    void onQKey() noexcept;
    void onSplit() noexcept;
    void onEject() noexcept;
    void onSpectate() noexcept;
    void onDisconnection() noexcept;
    void onTarget(const Vec2&) noexcept;
    void onSpawn(std::string name) noexcept;

    ~Player();
private:
    std::string _skinName = "";
    std::string _cellName = "An unnamed cell";
    
    float  scale         = 0.0f;
    double _score        = 0.0;
    float  filteredScale = 1.0f;

    Vec2 _mouse  = { 0, 0 };
    Vec2 _center = { 0, 0 };
    Rect viewBox = Rect(0, 0, 0, 0);

    // Pair entities with their nodeIds
    std::map<unsigned long long, e_ptr> visibleNodes;

    PlayerState _state = PlayerState::DEAD;
};
