#include "PacketHandler.h"
#include "GameServer.h"

PacketHandler::PacketHandler() {

}

void PacketHandler::sendPacket(std::vector<unsigned char> &packet) {
    _socket->send(reinterpret_cast<const char *>(packet.data()), packet.size(), uWS::BINARY);
}

// Packets
// reference for latest protocol: https://github.com/NuclearC/agar.io-protocol/
void PacketHandler::sendPkt_updateNodes() {
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

    _writer.clearBuffer();
    _writer.writeUInt8_LE(0x10);
	
    // Test for food
    _writer.writeUInt16_LE(0); // eatRecord
	
    for (int i = 0; i < _foods.size(); ++i) {
        Food *food = _foods[i];
        _writer.writeUInt32_LE(food->getNodeId()); // nodeId
        _writer.writeInt32_LE((int)food->getPosition().x); // x
        _writer.writeInt32_LE((int)food->getPosition().y); // y
        _writer.writeUInt16_LE((unsigned short)food->getSize()); // radius

        unsigned char flags = 0x80; // extendedFlags
        _writer.writeUInt8_LE(0x80); // flags

        if (flags & 0x80)
            _writer.writeUInt8_LE(0x01); // flags2

        if (flags & 0x02) {
            _writer.writeUInt8_LE(food->getColor().r); // red
            _writer.writeUInt8_LE(food->getColor().g); // green
            _writer.writeUInt8_LE(food->getColor().b); // blue
        }
    }
    _writer.writeUInt32_LE(0); // stop updateRecord

    _writer.writeUInt16_LE(0); // removeRecord
	
    sendPacket(_writer.getBuffer());
}
void PacketHandler::sendPkt_setup() {
    // TEMPORARY: updateNodes packet, but empty
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0x10);
    _writer.writeUInt16_LE(0); // eatRecord
    _writer.writeUInt32_LE(0); // stop updateRecord
    _writer.writeUInt16_LE(0); // removeRecord
    sendPacket(_writer.getBuffer());
}
void PacketHandler::sendPkt_updateViewport(const float &x, const float &y, const float &scale) {
    /*
    --Description--
    updateViewportPkt: {
        opCode,
        x,
        y,
        scale
    }
    --Data Types--
    updateViewportPkt: {
        uint8,
        float,
        float,
        float
    }
    */
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0x11);
    _writer.writeFloat_LE(x);
    _writer.writeFloat_LE(y);
    _writer.writeFloat_LE(scale);
    sendPacket(_writer.getBuffer());
}
void PacketHandler::sendPkt_clearAll() {
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0x12);
    sendPacket(_writer.getBuffer());
}
void PacketHandler::sendPkt_addNode(const unsigned int &nodeId) {
    /*
    --Description--
    addNodePkt: {
        opCode,
        nodeId
    }
    --Data Types--
    addNodePkt: {
        uint8,
        uint32
    }
    */
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0x20);
    _writer.writeUInt32_LE(nodeId);
    sendPacket(_writer.getBuffer());
}
void PacketHandler::sendPkt_leaderboardRgb(const unsigned int &length, const float &r, const float &g, const float &b) {
    /*
    --Description--
    leaderboardRGBPkt: {
        opCode,
        length,
        red,
        green,
        blue
    }
    --Data Types--
    leaderboardRGBPkt: {
        uint8,
        uint32,
        float,
        float,
        float
    }
    */
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0x32);
    _writer.writeUInt32_LE(length);
    _writer.writeFloat_LE(r);
    _writer.writeFloat_LE(g);
    _writer.writeFloat_LE(b);
    sendPacket(_writer.getBuffer());
}
void PacketHandler::sendPkt_leaderboardList() {
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
void PacketHandler::sendPkt_setBorder(const double &minx, const double &miny, const double &maxx, const double &maxy, const unsigned int &gameMode, const std::string &serverName) {
    /*
    --Description--
    setBorderPkt: {
        opCode,
        minx,
        miny,
        maxx,
        maxy,
        gameMode,
        serverName
    }
    --Data Types--
    setBorderPkt: {
        uint8,
        double,
        double,
        double,
        double,
        uint32,
        null_uft8
    }
    */
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0x40);
    _writer.writeDouble_LE(minx);
    _writer.writeDouble_LE(miny);
    _writer.writeDouble_LE(maxx);
    _writer.writeDouble_LE(maxy);
    _writer.writeUInt32_LE(gameMode);
    _writer.writeStr(serverName);
    sendPacket(_writer.getBuffer());
}
void PacketHandler::sendPkt_captchaRequest() {
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0x55);
    sendPacket(_writer.getBuffer());
}
void PacketHandler::sendPkt_logIn() {
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0x67);
    sendPacket(_writer.getBuffer());
}
void PacketHandler::sendPkt_logOut() {
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0x68);
    sendPacket(_writer.getBuffer());
}
void PacketHandler::sendPkt_playerBanned(const std::string &accountNameOrIp) {
    /*
    --Description--
    playerBannedPkt: {
        opCode,
        accountName/ip
    }
    --Data Types--
    playerBannedPkt: {
        uint8,
        null_uft8
    }
    */
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0x69);
    sendPacket(_writer.getBuffer());
}
void PacketHandler::sendPkt_outdatedClient() {
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0x80);
    sendPacket(_writer.getBuffer());
}
void PacketHandler::sendPkt_showArrow(const short &x, const short &y, const std::string &playerName) {
    /*
    --Description--
    showArrowPkt: {
        opCode,
        x,
        y,
        playerName
    }
    --Data Types--
    showArrowPkt: {
        uint8,
        int16,
        int16,
        null_str8
    }
    */
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0xa0);
    _writer.writeInt16_LE(x);
    _writer.writeInt16_LE(y);
    _writer.writeStr(playerName);
    sendPacket(_writer.getBuffer());
}
void PacketHandler::sendPkt_removeArrow() {
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0xa1);
    sendPacket(_writer.getBuffer());
}
void PacketHandler::sendPkt_ping() {
    /*
    --Description--
    pingPkt: {
        opCode,
        randomData
    }
    --Data Types--
    pingPkt: {
        uint8,
        ???
    }
    */
    _writer.clearBuffer();
    _writer.writeUInt8_LE(0xe2);
    sendPacket(_writer.getBuffer());
}

void PacketHandler::recievePacket(std::vector<unsigned char> &packet) {
    BinaryReader reader(packet);

    unsigned char opCode = reader.readUInt8_LE();

    switch (opCode) {
        // Spawn
        case 0x0: {
            std::cout << "Spawn packet recieved.\n";
            std::cout << "Name: " << reader.readStr(packet.size()) << "\n";
            break;
        }

        // Spectate
        case 0x01: {
            std::cout << "Spectate packet recieved.\n";
            break;
        }

        // Set Target
        case 0x10: {
            std::cout << "Set Target packet recieved.\n";

            _target = { 
                (double)reader.readInt32_LE(), 
                (double)reader.readInt32_LE()
            };
            //std::cout << "X: " << target.x << "\n";
            //std::cout << "Y: " << target.y << "\n";
            break;
        }

        // Split
        case 0x11: {
            std::cout << "Split packet recieved.\n";
			
            // test
            sendPkt_updateNodes();
            break;
        }

        // Eject Mass
        case 0x15: {
            std::cout << "Eject Mass packet recieved.\n";

            // Food test
            Food food = Food();
            _foods.push_back(&food);
            std::cout << "nodeId: " << _foods[food.getNodeId()-1]->getNodeId() << "\n";
            std::cout << "position: { " << _foods[food.getNodeId()-1]->getPosition().x << ", " << _foods[food.getNodeId()-1]->getPosition().y << " }\n";
            std::cout << "color: { " << _foods[food.getNodeId()-1]->getColor().r << ", " << _foods[food.getNodeId()-1]->getColor().g << ", " << _foods[food.getNodeId()-1]->getColor().b << " }\n";
            sendPkt_updateViewport(food.getPosition().x, food.getPosition().y, 0.4);
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

            _protocol = reader.readUInt32_LE();

            if (_protocol > 17) {
                _socket->close(1002, "Unsupported protocol");
                break;
            }

            std::cout << "Protocol version: " << _protocol << "\n\n";
            break;
        }

        // Connection Key
        case 0xff: {
            std::cout << "Connection Key packet recieved.\n";

            int key = reader.readInt32_LE();

            if (key != 0) {
                _socket->close(1002, "Unaccepted protocol");
                break;
            }

            double halfWidth = config<double>("mapWidth") / 2;
            double halfHeight = config<double>("mapHeight") / 2;

            sendPkt_clearAll();
            sendPkt_setBorder(-halfWidth, -halfHeight, halfWidth, halfHeight, 0, "MOE\0");
            sendPkt_setup();
            break;
        }
    }
}
