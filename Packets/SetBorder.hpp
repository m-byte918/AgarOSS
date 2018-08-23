#include "Packet.hpp"
#include "../Game/Map.hpp"

class SetBorder : public Packet {
public:
    SetBorder() {
        buffer.writeUInt8(0x40);

        buffer.writeDouble_LE(map::getBounds().left());
        buffer.writeDouble_LE(map::getBounds().bottom());
        buffer.writeDouble_LE(map::getBounds().right());
        buffer.writeDouble_LE(map::getBounds().top());
        buffer.writeUInt32_LE(cfg::game_mode);
        buffer.writeStr(cfg::server_name + "\0");
    }
    std::string toString() {
        Packet::toString();
        ss << "\nMap: {"
           << "\n    left: " << buffer.readDouble_LE()
           << "\n    bottom: " << buffer.readDouble_LE()
           << "\n    right: " << buffer.readDouble_LE()
           << "\n    top: " << buffer.readDouble_LE()
           << "\n}"
           << "\nGamemode: " << buffer.readUInt32_LE()
           << "\nServerName: " << buffer.readStr();
        return ss.str();
    }
};