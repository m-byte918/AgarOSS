#pragma once
#include "../Connection/PacketHandler.hpp"

class Player;
class Protocol {
public:
    Buffer buffer;

    Protocol(Player *owner);

    virtual Buffer &addNode(unsigned int nodeId);
    virtual Buffer &banPlayer(const std::string &playerNameOrIp);
    virtual Buffer &clearAll();
    virtual Buffer &clearOwned();
    virtual Buffer &compressed(std::vector<unsigned char> &_Packet);
    virtual Buffer &dks2(int _dks2);
    virtual Buffer &login();
    virtual Buffer &logout();
    virtual Buffer &mobileData();
    virtual Buffer &ping();
    virtual Buffer &removeArrow();
    virtual Buffer &requestCaptcha();
    virtual Buffer &requestClientUpdate();
    virtual Buffer &setBorder();
    virtual Buffer &showArrow(const Vec2 &position, const std::string &playerName);
    virtual Buffer &updateLeaderboardList();
    virtual Buffer &updateLeaderboardRGB(const std::vector<float> &board);
    virtual Buffer &updateLeaderboardText(const std::vector<std::string> &board);
    virtual Buffer &updateNodes(const std::vector<e_ptr> &eatNodes, const std::vector<e_ptr> &updNodes,
        const std::vector<e_ptr> &delNodes, const std::vector<e_ptr> &addNodes);
    virtual Buffer &updateViewport(const Vec2 &position, float scale);
    virtual Buffer &chatMessage(/**/);
    virtual Buffer &drawLine(const Vec2 &position);
    virtual Buffer &serverStat(const std::string &info);
    virtual Buffer &auth(const std::string &str);

    virtual ~Protocol();
protected:
    Player *player = nullptr;
};