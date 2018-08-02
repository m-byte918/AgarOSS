#include "Packet.hpp"

class RequestCaptcha : public Packet {
public:
    RequestCaptcha() {
        buffer.writeUInt8(0x55);
    }
};