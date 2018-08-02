#include "Packet"

class BanPlayer : public Packet {
public:
    BanPlayer(const std::string &playerNameOrIp) {
        buffer.writeUInt8(0x69);
        buffer.writeStr(accountNameOrIp + "\0");
    }
    std::string toString() {
        Packet::toString();
        ss << "\nAccount/IP: " buffer.readStr();
        return ss.str();
    }
};