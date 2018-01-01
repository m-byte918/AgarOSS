#pragma once

#include <iostream>
#include <uWS/uWS.h>

#include "Modules/BinaryRW.h"
#include "Entity/EntityHandler.h"
#include "Entity/Entity.h"

class PacketHandler {
    private:
        BinaryWriter _writer;
        unsigned int _protocol;

    public:
        Position _target = { 0, 0 };

        uWS::WebSocket<uWS::SERVER> *_socket;

        PacketHandler();

        void sendPacket(std::vector<unsigned char> &packet);

        // Packets
        void sendPkt_updateNodes();
        void sendPkt_setup();
        void sendPkt_updateViewport(const float &x, const float &y, const float &scale);
        void sendPkt_clearAll();
        void sendPkt_addNode(const unsigned int &nodeId);
        void sendPkt_leaderboardRgb(const unsigned int &length, const float &r, const float &g, const float &b);
        void sendPkt_leaderboardList();
        void sendPkt_setBorder(const double &minx, const double &miny, const double &maxx, const double &maxy, const unsigned int &gameMode, const std::string &serverName);
        void sendPkt_captchaRequest();
        void sendPkt_logIn();
        void sendPkt_logOut();
        void sendPkt_playerBanned(const std::string &accountNameOrIp);
        void sendPkt_outdatedClient();
        void sendPkt_showArrow(const short &x, const short &y, const std::string &playerName);
        void sendPkt_removeArrow();
        void sendPkt_ping();

        void recievePacket(std::vector<unsigned char> &packet);
};
