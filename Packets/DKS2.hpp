#include "Packet"

class DKS2 : public Packet {
public:
    DKS2(int dks2) {
        buffer.writeUInt8(0xf1);
        buffer.writeInt32_LE(dks2);
    }
    std::string toString() {
        Packet::toString();
        ss << "\nDKS2: " << buffer.readInt32_LE();
        return ss.str();
    }
};