#include "Packet.hpp"

class AddNode : public Packet {
public:
    AddNode(unsigned int nodeId) {
        buffer.writeUInt8(0x20);
        buffer.writeUInt32_LE(nodeId);
    }
    std::string toString() {
        Packet::toString();
        ss << "\nNodeId: " << buffer.readUInt32_LE();
        return ss.str();
    }
};