#include "Packet.hpp"

class ClearAll : public Packet {
public:
    ClearAll() {
        buffer.writeUInt8(0x12);
    }
};