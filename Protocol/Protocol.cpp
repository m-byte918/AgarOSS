#include "Protocol.hpp"
#include "../Game/Map.hpp"

Protocol::Protocol(Player *owner) :
    player(owner) {
}

// Same for each protocol
Buffer &Protocol::addNode(unsigned int nodeId) {
    buffer.writeUInt8(0x20);
    return buffer.writeUInt32_LE(nodeId);
}
Buffer &Protocol::banPlayer(const std::string &playerNameOrIp) {
    buffer.writeUInt8(0x69);
    return buffer.writeStr(playerNameOrIp + "\0");
}
Buffer &Protocol::clearAll() {
    return buffer.writeUInt8(0x12);
}
Buffer &Protocol::clearOwned() {
    return buffer.writeUInt8(0x14);
}
Buffer &Protocol::compressed(std::vector<unsigned char> &_Packet) {
    buffer.setBuffer(_Packet);
    buffer.writeUInt8(0xff);
    buffer.writeUInt32_LE((unsigned int)_Packet.size());

    for (const unsigned char &byte : _Packet)
        buffer.writeUInt8(byte);
    return buffer;
}
Buffer &Protocol::dks2(int _dks2) {
    buffer.writeUInt8(0xf1);
    return buffer.writeInt32_LE(_dks2);
}
Buffer &Protocol::login() {
    return buffer.writeUInt8(0x67);
}
Buffer &Protocol::logout() {
    return buffer.writeUInt8(0x68);
}
Buffer &Protocol::mobileData() {
    return buffer.writeUInt8(0x66);
}
Buffer &Protocol::ping() {
    return buffer.writeUInt8(0xe2);
}
Buffer &Protocol::removeArrow() {
    return buffer.writeUInt8(0xa1);
}
Buffer &Protocol::requestCaptcha() {
    return buffer.writeUInt8(0x55);
}
Buffer &Protocol::requestClientUpdate() {
    return buffer.writeUInt8(0x80);
}
Buffer &Protocol::setBorder() {
    buffer.writeUInt8(0x40);

    buffer.writeDouble_LE(map::bounds().left());
    buffer.writeDouble_LE(map::bounds().bottom());
    buffer.writeDouble_LE(map::bounds().right());
    return buffer.writeDouble_LE(map::bounds().top());
}
Buffer &Protocol::showArrow(const Vec2 &position, const std::string &playerName) {
    buffer.writeUInt8(0xa0);
    buffer.writeInt16_LE((short)position.x);
    buffer.writeInt16_LE((short)position.y);
    return buffer.writeStr(playerName + "\0");
}
// Update these for each protocol
Buffer &Protocol::updateLeaderboardList() {
    return buffer.writeUInt8(0x35);
}
// Update these for each protocol
Buffer &Protocol::updateLeaderboardRGB(const std::vector<float> &board) {
    buffer.writeUInt8(0x32);
    buffer.writeUInt32_LE((unsigned int)board.size());
    for (const float &color : board)
        buffer.writeFloat_LE(color);
    return buffer;
}
// Update these for each protocol
Buffer &Protocol::updateNodes(const std::vector<e_ptr> &eatNodes, const std::vector<e_ptr> &updNodes,
    const std::vector<e_ptr> &delNodes, const std::vector<e_ptr> &addNodes) {
    addNodes;
    delNodes;
    updNodes;
    buffer.writeUInt8(0x10);

    // Eat record
    buffer.writeUInt16_LE((unsigned short)eatNodes.size());
    for (const e_ptr &entity : eatNodes) {
        buffer.writeUInt32_LE(entity->killerId());
        buffer.writeUInt32_LE((unsigned)entity->nodeId());
    }
    return buffer;
}
Buffer &Protocol::updateViewport(const Vec2 &position, float scale) {
    buffer.writeUInt8(0x11);
    buffer.writeFloat_LE((float)position.x);
    buffer.writeFloat_LE((float)position.y);
    return buffer.writeFloat_LE(scale);
}
Buffer &Protocol::chatMessage(/**/) {
    buffer.writeUInt8(0x63);
    // to be implemented
    return buffer;
}
Buffer &Protocol::drawLine(const Vec2 &position) {
    buffer.writeUInt8(0x15);
    buffer.writeInt16_LE((short)position.x);
    return buffer.writeInt16_LE((short)position.y);
}
Buffer &Protocol::serverStat(const std::string &info) {
    info;
    return buffer;
}
Buffer &Protocol::auth(const std::string &str) {
    str;
    return buffer;
}

/*
Usertext:     0x30 (48), 4-13
Usertext/Reg: 0x31 (49), 4-10
Reg:          0x33 (51), 11-13
Reg:          0x34 (52), 13
Usertext/Reg: 0x35 (53), 14-18
Reg:          0x36 (54), 15-16
*/