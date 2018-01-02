#pragma once

#include <iostream>
#include <uWS/uWS.h>

#include "Modules/BinaryRW.h"
#include "Entity/Entity.h"

class Player;
class Packet {
    private:
        BinaryWriter _writer;
        unsigned int _protocol;

    public:
        Player *_player;

        Packet();

        // Packet sending
        void sendPacket(std::vector<unsigned char> &packet);

        void sendUpdateNodes();
        void sendSetup();
        void sendUpdateViewport(const float &x, const float &y, const float &scale);
        void sendClearAll();
        void sendAddNode(const unsigned int &nodeId);
        void sendLeaderboardRgb(const unsigned int &length, const float &r, const float &g, const float &b);
        void sendLeaderboardList();
        void sendSetBorder();
        void sendCaptchaRequest();
        void sendLogIn();
        void sendLogOut();
        void sendPlayerBanned(const std::string &accountNameOrIp);
        void sendOutdatedClient();
        void sendShowArrow(const short &x, const short &y, const std::string &playerName);
        void sendRemoveArrow();
        void sendPing();

        // Packet recieving
        void onPacket(std::vector<unsigned char> &packet);

        void onSpawn(const std::string &name);
        void onSpectate();
        void onTarget(const Position &target);
        void onQkey();
        void onConnection(unsigned int protocol);
        void onConnectionKey(int key);
};

#include "Player.h" // this might be wrong
