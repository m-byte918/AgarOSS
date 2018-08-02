#include "Packet"

class Ping : public Packet {
public:
    Ping() {
        buffer.writeUInt8(0xe2);
    }
};