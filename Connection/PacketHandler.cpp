#include "PacketHandler.hpp"
#include "../Game/Map.hpp"
#include "../Game/Game.hpp"
#include "../Player/Player.hpp"
#include "../Modules/Utils.hpp"
#include "../Modules/Logger.hpp"
#include "../Protocol/Protocol.hpp"
#include "../Protocol/Protocol_4.hpp"
#include "../Protocol/Protocol_5.hpp"
#include "../Protocol/Protocol_6.hpp"
#include "../Protocol/Protocol_7.hpp"
#include "../Protocol/Protocol_8.hpp"
#include "../Protocol/Protocol_9.hpp"
#include "../Protocol/Protocol_10.hpp"
#include "../Protocol/Protocol_11.hpp"
#include "../Protocol/Protocol_12.hpp"
#include "../Protocol/Protocol_13.hpp"
#include "../Protocol/Protocol_14.hpp"
#include "../Protocol/Protocol_15.hpp"
#include "../Protocol/Protocol_16.hpp"
#include "../Protocol/Protocol_17.hpp"
#include "../Protocol/Protocol_18.hpp"

PacketHandler::PacketHandler(Player *owner) :
    player(owner) {
}

void PacketHandler::sendPacket(Buffer &buffer) const {
    const std::vector<unsigned char> &buf = buffer.getBuffer();
    if (buf.empty() || !player->socket || player->socket->isClosed())
        return;
    player->socket->send(reinterpret_cast<const char *>(buf.data()), buf.size(), uWS::BINARY);
    buffer.clear();
}

void PacketHandler::onPacket(std::vector<unsigned char> &packet) {
    Buffer buffer(packet);

    // Process OpCode
    switch ((OpCode)buffer.readUInt8()) {
    case OpCode::SPAWN:
        onSpawn(buffer.readStr());
        break;
    case OpCode::SPECTATE:
        onSpectate();
        break;
    case OpCode::SET_TARGET:
        onTarget({ (double)buffer.readInt32_LE(), (double)buffer.readInt32_LE() });
        break;
    case OpCode::SPLIT:
        onSplit();
        break;
    case OpCode::QKEY_PRESSED:
        onQKey();
        break;
    case OpCode::EJECT:
        onEject();
        break;
    case OpCode::CAPTCHA_RESPONSE:
        Logger::info("Captcha Response packet recieved.");
        break;
    case OpCode::PONG:
        Logger::info("Pong packet recieved.");
        break;
    case OpCode::ESTABLISH_CONNECTION:
        onEstablishedConnection(buffer.readUInt32_LE());
        break;
    case OpCode::CONNECTION_KEY:
        onConnectionKey();
        break;
    }
}

void PacketHandler::onSpawn(std::string name) const noexcept {
    Logger::info("Spawn packet recieved.");
    player->onSpawn(name);
}

void PacketHandler::onSpectate() const noexcept {
    Logger::info("Spectate packet recieved.");
    player->onSpectate();
}
void PacketHandler::onTarget(const Vec2 &mouse) const noexcept {
    //Logger::info("Set Target packet recieved.");
    player->onTarget(mouse);
}
void PacketHandler::onSplit() const noexcept {
    player->onSplit();
}
void PacketHandler::onQKey() const noexcept {
    Logger::info("Q keypress packet recieved.");
    player->onQKey();
}
void PacketHandler::onEject() const noexcept {
    player->onEject();
}
void PacketHandler::onEstablishedConnection(const unsigned &protocol) const noexcept {
    Logger::info("Establish Connection packet recieved.");
    Logger::info("Protocol version: " + std::to_string(protocol));
    if (protocol < cfg::server_minSupportedProtocol ||
        protocol > cfg::server_maxSupportedProtocol) {
        player->socket->close(1002, "Unsupported protocol");
        return;
    }
    switch (protocol) {
        case 4:  { player->protocol = new Protocol_4(player);  break; }
        case 5:  { player->protocol = new Protocol_5(player);  break; }
        case 6:  { player->protocol = new Protocol_6(player);  break; }
        case 7:  { player->protocol = new Protocol_7(player);  break; }
        case 8:  { player->protocol = new Protocol_8(player);  break; }
        case 9:  { player->protocol = new Protocol_9(player);  break; }
        case 10: { player->protocol = new Protocol_10(player); break; }
        case 11: { player->protocol = new Protocol_11(player); break; }
        case 12: { player->protocol = new Protocol_12(player); break; }
        case 13: { player->protocol = new Protocol_13(player); break; }
        case 14: { player->protocol = new Protocol_14(player); break; }
        case 15: { player->protocol = new Protocol_15(player); break; }
        case 16: { player->protocol = new Protocol_16(player); break; }
        case 17: { player->protocol = new Protocol_17(player); break; }
        case 18: { player->protocol = new Protocol_18(player); break; }
        default: { player->protocol = new Protocol(player); }
    }
    player->protocolNum = protocol;
}
void PacketHandler::onConnectionKey() const noexcept {
    Logger::info("Connection Key packet recieved.");
    sendPacket(player->protocol->clearAll());
    sendPacket(player->protocol->setBorder());
}

PacketHandler::~PacketHandler() {
}