#include "PacketHandler.hpp"
#include "../Player.hpp"
#include "../Modules/Logger.hpp"

#include "Packet.hpp"
#include "ClearAll.hpp"
#include "SetBorder.hpp"

PacketHandler::PacketHandler(Player *owner):
    player(owner) {
}

void PacketHandler::sendPacket(Packet &packet) const {
    const std::vector<unsigned char> &buffer = packet.toBuffer();
    if (buffer.empty() || player->socket->isClosed())
        return;
    player->socket->send(reinterpret_cast<const char *>(buffer.data()), buffer.size(), uWS::BINARY);
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

void PacketHandler::onSpawn(std::string &name) const noexcept {
    Logger::info("Spawn packet recieved.");
    player->onSpawn(name);
}

void PacketHandler::onSpectate() const noexcept {
    Logger::info("Spectate packet recieved.");
    player->onSpectate();
}
void PacketHandler::onTarget(const Vector2 &mouse) const noexcept {
    //Logger::info("Set Target packet recieved.");
    player->onTarget(mouse);
}
void PacketHandler::onSplit() const noexcept {
    Logger::info("Split packet recieved.");
    player->onSplit();
}
void PacketHandler::onQKey() const noexcept {
    Logger::info("Q keypress packet recieved.");
    player->onQKey();
}
void PacketHandler::onEject() const noexcept {
    Logger::info("Eject Mass packet recieved.");
    player->onEject();
}
void PacketHandler::onEstablishedConnection(const unsigned &protocol) const noexcept {
    Logger::info("Establish Connection packet recieved.");
    Logger::info("Protocol version: " + std::to_string(protocol));
    if (protocol < (int)config["server"]["minSupportedProtocol"] || 
        protocol > (int)config["server"]["maxSupportedProtocol"]) {
        player->socket->close(1002, "Unsupported protocol");
        return;
    }
    player->protocol = protocol;
}
void PacketHandler::onConnectionKey() const noexcept {
    Logger::info("Connection Key packet recieved.");
    sendPacket(ClearAll());
    sendPacket(SetBorder());
}

PacketHandler::~PacketHandler() {
}
