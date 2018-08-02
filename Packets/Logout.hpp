#include "Packet"

class Logout : public Packet {
public:
    Logout() {
        buffer.writeUInt8(0x68);
    }
};