#include "Packet"

class RequestClientUpdate : public Packet {
public:
    RequestClientUpdate() {
        buffer.writeUInt8(0x80);
    }
};