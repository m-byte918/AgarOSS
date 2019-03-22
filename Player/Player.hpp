#pragma once
#include "../Game/Game.hpp"
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
class Minion;

class Player {
public:
    // Server, protocol
    Player       *owner    = nullptr;
    Server       *server   = nullptr;
    Protocol     *protocol = nullptr;
    PacketHandler packetHandler{nullptr};
    uWS::WebSocket<uWS::SERVER> *socket;

    // Misc
    unsigned int       id;
    unsigned int       protocolNum        = 0;
    unsigned long long disconnectionTick  = 0;
    bool               isForceMerging     = false;
    bool               controllingMinions = false;
    float              spawnRadius        = cfg::playerCell_baseRadius;
    std::vector<Entity*> cells;
    std::vector<Minion*> minions;

    // Setters
    Player(Server *_server);
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
    virtual void update(unsigned long long tick);
    void updateScore();
    void updateCenter();
    void updateViewBox();
    void updateVisibleNodes();
    virtual void updateDisconnection(unsigned long long tick);

    // Recieved information
    void onQKey() noexcept;
    void onSplit() noexcept;
    void onEject() noexcept;
    void onSpectate() noexcept;
    virtual void onDisconnection() noexcept;
    void onTarget(const Vec2&) noexcept;
    void onSpawn(std::string name) noexcept;

    // Misc
    void spawn(std::string name) noexcept;

    virtual ~Player();
private:
    std::string _skinName = "";
    std::string _cellName = "";
    
    float  scale         = 0.0f;
    float  filteredScale = 1.0f;
    double _score        = 0.0;

    Vec2 _center = { 0, 0 };
    Rect viewBox = Rect(0, 0, 0, 0);

    // Pair entities with their nodeIds
    std::map<unsigned long long, e_ptr> visibleNodes;

protected:
    Vec2        _mouse = { 0, 0 };
    PlayerState _state = PlayerState::DEAD;
};
