#include "Packet.hpp"

class UpdateLeaderboardRGB : public Packet {
public:
    UpdateLeaderboardRGB(const std::vector<float> &board) {
        buffer.writeUInt8(0x32);
        buffer.writeUInt32_LE(board.size());
        for (const float &color : board)
            buffer.writeFloat_LE(color);
    }
    std::string toString() {
        Packet::toString();
        ss << "\nBoard: {"
           << "\n    size: " << buffer.readUInt32_LE()
           << "\n    R: " << buffer.readFloat_LE()
           << "\n    G: " << buffer.readFloat_LE()
           << "\n    B: " << buffer.readFloat_LE()
           << "\n}";
        return ss.str();
    }
};