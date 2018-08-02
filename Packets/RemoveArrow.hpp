#include "Packet"

class RemoveArrow : public Packet {
public:
    RemoveArrow() {
        buffer.writeUInt8(0xa1);
    }
};