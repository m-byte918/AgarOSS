#include "Packet"

class Compressed : public Packet {
public:
    Compressed(const Packet &_Packet):
        packet(_Packet.toBuffer()) {
        buffer.writeUInt8(0xff);
        buffer.writeUInt32_LE(packet.size());

        for (const unsigned char &byte : packet)
            buffer.writeUInt8(byte);
    }
    std::string toString() {
        Packet::toString();
        unsigned int size = buffer.readUInt32_LE();
        ss << "\nDecompressed Length: " << size;

        if (size > 0) {
            ss << "\nBytes: {";
            for (unsigned long long i = 0; i < packet.size(); ++i)
                ss << "\n   " << i << ": " << std::hex << std::setw(2) << packet[i];
            ss << "\n}";
        }
        return ss.str();
    }
private:
    std::vector<unsigned char> packet;
};