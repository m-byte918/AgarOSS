#include "Packet"

class Login : public Packet {
public:
    Login() {
        buffer.writeUInt8(0x67);
    }
};