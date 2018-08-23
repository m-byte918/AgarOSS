#include "Packet"
#include "../Modules/Utils.hpp"

class ShowArrow : public Packet {
public:
    ShowArrow(const Vector2 &position, const std::string &playerName) {
        buffer.writeUInt8(0xa0);
        buffer.writeInt16_LE((short)position.x);
        buffer.writeInt16_LE((short)position.y);
        buffer.writeStr(playerName + "\0");
    }
    std::string toString() {
        Packet::toString();
        ss << "\nX: " << buffer.readInt16_LE()
           << "\nY: " << buffer.readInt16_LE()
           << "\nPlayerName: " << buffer.readStr();
        return ss.str();
    }
};