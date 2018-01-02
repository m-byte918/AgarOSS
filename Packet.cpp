#include "Packet.h"

Packet::Packet() {

}

void Packet::sendPacket(std::vector<unsigned char> &packet) {
    _player->_socket->send(reinterpret_cast<const char *>(packet.data()), packet.size(), uWS::BINARY);
}

// Packets
// reference for latest protocol: https://github.com/NuclearC/agar.io-protocol/
void Packet::sendUpdateNodes() {
    /* 
    --Description--
    updateNodesPkt: { 
        opCode, 
        eatRecordLength, 
        eatRecord: { 
            eaterID, 
            victimID 
        }, 
        updateRecord: { 
            id, 
            x, 
            y, 
            radius, 
            flags: { 
                virus, 
                hasColor, 
                hasSkin, 
                hasName, 
                agitated, 
                ejectedMass, 
                othersEjectedMass, 
                extendedFlags 
            }, 
            flags2: { 
                food, 
                isFriend, 
                hasAccountID 
            }, 
            red, 
            green, 
            blue, 
            skin, 
            name, 
            account 
        }, 
        removeRecordLength, 
        removeRecord: { id }
    }
    --Data Types--
    updateNodesPkt: {
        uint8,
        uint16,
        eatRecord: {
            uint32,
            uint32
        },
        updateRecord: { 
            uint32, 
            int32, 
            int32, 
            uint16, 
            (uint8)flags: { 
                0x01, 
                0x02, 
                0x04, 
                0x08, 
                0x10, 
                0x20, 
                0x40, 
                0x80 
            },
            (uint8)flags2: { 
                0x01, 
                0x02, 
                0x04 
            }, 
            uint8, 
            uint8, 
            uint8, 
            null_utf8, 
            null_utf8, 
            uint32 
        }, 
        uint16, 
        removeRecord: { uint32 }
    }*/

    // Test for food
    for (int i = 0; i < _foods.size(); ++i) {
        Food *food = _foods[i];
        _writer.clearBuffer();
        _writer.writeUInt8_LE(0x10);
        _writer.writeUInt16_LE(0); // eatRecord
        _writer.writeUInt32_LE(food->getNodeId()); // nodeId
        _writer.writeInt32_LE((int)food->getPosition().x); // x
        _writer.writeInt32_LE((int)food->getPosition().y); // y
        _writer.writeUInt16_LE((unsigned short)food->getSize()); // radius

        unsigned char flags = 0; // extendedFlags
        flags |= 0x02; // has color
        flags |= 0x80; // extended flags
        _writer.writeUInt8_LE(flags); // flags

        if (flags & 0x80)
            _writer.writeUInt8_LE(0x01); // flags2

        if (flags & 0x02) {
            _writer.writeUInt8_LE(food->getColor().r); // red
            _writer.writeUInt8_LE(food->getColor().g); // green
            _writer.writeUInt8_LE(food->getColor().b); // blue
        }
        _writer.writeUInt32_LE(0); // stop updateRecord
        _writer.writeUInt16_LE(0); // removeRecord
        sendPacket(_writer.getBuffer());
    }
}
void Packet::sendUpdateViewport(const float &x, const float &y, const float &scale) {
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0x11);
    _writer.writeFloat_LE(x);
    _writer.writeFloat_LE(y);
    _writer.writeFloat_LE(scale);
    sendPacket(_writer.getBuffer());
}
void Packet::sendClearAll() {
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0x12);
    sendPacket(_writer.getBuffer());
}
void Packet::sendAddNode(const unsigned int &nodeId) {
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0x20);
    _writer.writeUInt32_LE(nodeId);
    sendPacket(_writer.getBuffer());
}
void Packet::sendLeaderboardRgb(const unsigned int &length, const float &r, const float &g, const float &b) {
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0x32);
    _writer.writeUInt32_LE(length);
    _writer.writeFloat_LE(r);
    _writer.writeFloat_LE(g);
    _writer.writeFloat_LE(b);
    sendPacket(_writer.getBuffer());
}
void Packet::sendLeaderboardList() {
    /*
    --Description--
    leaderboardListPkt: {
        opCode,
        friendCount,
        lbRecord: {
            flags: { 
                place,
                name,
                accountId,
                isMe,
                isFriend
            },
            place,
            name,
            account
        }
    }
    --Data Types--
    leaderboardListPkt: {
        uint8,
        uint16,
        lbRecord: {
            (uint8)flags: { 
                0x01,
                0x02,
                0x04,
                0x08,
                0x10
            },
            uint16,
            null_uft8,
            uint32
        }
    }
    */
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0x35);
    sendPacket(_writer.getBuffer());
}
void Packet::sendSetBorder() {
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0x40);
    _writer.writeDouble_LE(_player->_owner->_border.minx);
    _writer.writeDouble_LE(_player->_owner->_border.miny);
    _writer.writeDouble_LE(_player->_owner->_border.maxx);
    _writer.writeDouble_LE(_player->_owner->_border.maxy);
    _writer.writeUInt32_LE(config<unsigned int>("gameMode"));
    _writer.writeStr(config<std::string>("serverName") + "\0");
    sendPacket(_writer.getBuffer());
}
void Packet::sendCaptchaRequest() {
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0x55);
    sendPacket(_writer.getBuffer());
}
void Packet::sendLogIn() {
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0x67);
    sendPacket(_writer.getBuffer());
}
void Packet::sendLogOut() {
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0x68);
    sendPacket(_writer.getBuffer());
}
void Packet::sendPlayerBanned(const std::string &accountNameOrIp) {
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0x69);
    _writer.writeStr(accountNameOrIp + "\0");
    sendPacket(_writer.getBuffer());
}
void Packet::sendOutdatedClient() {
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0x80);
    sendPacket(_writer.getBuffer());
}
void Packet::sendShowArrow(const short &x, const short &y, const std::string &playerName) {
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0xa0);
    _writer.writeInt16_LE(x);
    _writer.writeInt16_LE(y);
    _writer.writeStr(playerName + "\0");
    sendPacket(_writer.getBuffer());
}
void Packet::sendRemoveArrow() {
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0xa1);
    sendPacket(_writer.getBuffer());
}
void Packet::sendPing() {
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0xe2);
    sendPacket(_writer.getBuffer());
}

void Packet::onPacket(std::vector<unsigned char> &packet) {
    BinaryReader reader(packet);

    unsigned char opCode = reader.readUInt8_LE();

    switch (opCode) {
        // Spawn
        case 0x0: {
            std::cout << "Spawn packet recieved.\n";
            onSpawn(reader.readStr());
            break;
        }

        // Spectate
        case 0x01: {
            std::cout << "Spectate packet recieved.\n";
            onSpectate();
            break;
        }

        // Set Target
        case 0x10: {
            std::cout << "Set Target packet recieved.\n";
            onTarget({ (double)reader.readInt32_LE(), (double)reader.readInt32_LE() });
            break;
        }

        // Split
        case 0x11: {
            std::cout << "Split packet recieved.\n";
            break;
        }

        // Q key
        case 0x12: {
            std::cout << "Q keypress packet recieved.\n";
            onQkey();
            break;
        }

        // Eject Mass
        case 0x15: {
            std::cout << "Eject Mass packet recieved.\n";
            // Food test
            sendUpdateViewport(_player->_mouse.x, _player->_mouse.y, 0.1);
            break;
        }

        // Captcha Response
        case 0x56: {
            std::cout << "Captcha Response packet recieved.\n";
            break;
        }

        // Pong
        case 0xe3: {
            std::cout << "Pong packet recieved.\n";
            break;
        }

        // Establish Connection
        case 0xfe: {
            std::cout << "Establish Connection packet recieved.\n";
            onConnection(reader.readUInt32_LE());
            break;
        }

        // Connection Key
        case 0xff: {
            std::cout << "Connection Key packet recieved.\n";
            onConnectionKey(reader.readInt32_LE());
            break;
        }
    }
}

void Packet::onSpawn(const std::string &name) {
    unsigned short maxLen = config<unsigned short>("maxNameLength");
    std::string skin;

    short skinStart = name.find('<');
    short skinEnd   = name.find('>');

    if (skinStart != skinEnd && skinStart != -1 && skinEnd != -1)
        skin = name.substr(skinStart + 1, skinEnd - 1);

    _player->setSkin(skin.substr(0, maxLen));
    _player->setName(name.substr(skin.size() == 0 ? 0 : skinEnd + 1, maxLen));

    _player->setState(PlayerState::PLAYING);
}
void Packet::onSpectate() {
    _player->setState(PlayerState::SPECTATING);
}
void Packet::onTarget(const Position &target) {
    if (_player->_mouse != target)
        _player->_mouse = target;
}
void Packet::onQkey() {
    if (_player->getState() == PlayerState::SPECTATING) {
        _player->setState(PlayerState::FREEROAM);
    }
    else if (_player->getState() == PlayerState::FREEROAM) {
        _player->setState(PlayerState::SPECTATING);
    }
}
void Packet::onConnection(unsigned int protocol) {
    if (protocol > 17) {
        _player->_socket->close(1002, "Unsupported protocol");
        return;
    }
    _protocol = protocol;
    std::cout << "Protocol version: " << _protocol << "\n\n";
}
void Packet::onConnectionKey(int key) {
    if (key != 0) {
        _player->_socket->close(1002, "Unaccepted protocol");
        return;
    }
    sendClearAll();
    sendSetBorder();
    sendUpdateNodes(); // test
}
