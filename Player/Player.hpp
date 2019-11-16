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
class PlayerBot;

class Player {
public:
    // Server, protocol
    Player       *owner    = nullptr;
    Server       *server   = nullptr;
    Protocol     *protocol = nullptr;
    PacketHandler packetHandler{nullptr};
    uWS::WebSocket<false, true> *socket = nullptr;

    // Misc
    unsigned int       id                 = 0;
    unsigned int       protocolNum        = 0;
    bool               isForceMerging     = false;
    bool               controllingMinions = false;
    float              spawnRadius        = cfg::playerCell_baseRadius;
    std::vector<sptr<PlayerCell::Entity>> cells;
    std::vector<Minion*> minions;

    // Setters
    Player(Server *_server);
    void setDead() noexcept;
    void setFreeroam() noexcept;
    void setSpectating() noexcept;
    void setFullName(std::string name, bool isUCS2 = false) noexcept;
    void setSkinName(const std::string &name) noexcept;
    void setCellName(const std::string &name, bool isUCS2 = false) noexcept;
    
    // Getters
    double score() const noexcept;
    const Vec2 &mouse() const noexcept;
    const Vec2 &center() const noexcept;
    const PlayerState &state() const noexcept;
    const std::string &skinName() const noexcept;
    const std::string &cellNameUTF8() const noexcept;
    const std::string &cellNameUCS2() const noexcept;

    // Updating
    virtual void update();
    void updateScore();
    void updateCenter();
    void updateViewBox();
    virtual void updateVisibleNodes();

    // Recieved information
    void onQKey() noexcept;
    void onSplit() noexcept;
    void onEject() noexcept;
    void onSpectate() noexcept;
    void onDisconnection() noexcept;
    void onTarget(const Vec2&) noexcept;
    void onSpawn() noexcept;

    // Misc
    void spawn() noexcept;
    std::string toString() noexcept;

    virtual ~Player();

private:
    std::string _skinName = "";
    
    float         scale         = 0.0f;
    float         filteredScale = 1.0f;
    unsigned char lbUpdateTick  = 0;

    // Pair entities with their nodeIds
    std::map<unsigned int, e_ptr> visibleNodes;

protected:
    std::string _cellNameUCS2 = "";
    std::string _cellNameUTF8 = "";

    Rect viewBox = Rect(0, 0, cfg::player_viewBoxWidth, cfg::player_viewBoxHeight);

    double      _score  = 0.0;
    Vec2        _mouse  = { 0, 0 };
    Vec2        _center = { 0, 0 };
    PlayerState _state  = PlayerState::DEAD;
};
