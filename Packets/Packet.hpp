#pragma once

#include <sstream> // toString(s)
#include "../Modules/Buffer.hpp"

class Packet {
public:
    const std::vector<unsigned char> &toBuffer() const noexcept {
        return buffer.getBuffer();
    }
    std::string toString() noexcept {
        buffer.setReadOffset(0); // reset
        ss << "OpCode: " << (int)buffer.readUInt8();
        return ss.str();
    }
protected:
    Buffer buffer;
    std::stringstream ss;
};