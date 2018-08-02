#include "Packet"

class MobileData : public Packet {
public:
    MobileData() {
        buffer.writeUInt8(0x66);
    }
};