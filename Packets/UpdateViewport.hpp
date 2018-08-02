#include "Packet.hpp"
#include "../Modules/Utils.hpp"

class UpdateViewport : public Packet {
public:
    UpdateViewport(const Vector2 &position, float scale) {
        buffer.writeUInt8(0x11);
        buffer.writeFloat_LE((float)position.x);
        buffer.writeFloat_LE((float)position.y);
        buffer.writeFloat_LE(scale);
    }
    std::string toString() {
        Packet::toString();
        ss << "\nX: " << buffer.readFloat_LE()
           << "\nY: " << buffer.readFloat_LE()
           << "\nScale: " << buffer.readFloat_LE();
        return ss.str();
    }
};