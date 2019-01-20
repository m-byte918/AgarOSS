#pragma once
#include <vector>  // buffers
#include <sstream> // strings, byteStr()

class Buffer {
public:
    Buffer() noexcept;
    Buffer(const std::vector<unsigned char>&) noexcept;

    void setBuffer(const std::vector<unsigned char>&) noexcept;
    const std::vector<unsigned char> &getBuffer() const noexcept;
    void clear() noexcept;

    std::string byteStr(bool LE = true) const noexcept;

    /************************** Writing ***************************/

    template <class T> inline Buffer &writeBytes(const T &val, bool LE = true);
    unsigned long long getWriteOffset() const noexcept;

    Buffer &writeBool(bool) noexcept;
    Buffer &writeStr(const std::string&) noexcept;
    Buffer &writeStrNull(const std::string&) noexcept;
    Buffer &writeInt8(char) noexcept;
    Buffer &writeUInt8(unsigned char) noexcept;

    Buffer &writeInt16_LE(short) noexcept;
    Buffer &writeInt16_BE(short) noexcept;
    Buffer &writeUInt16_LE(unsigned short) noexcept;
    Buffer &writeUInt16_BE(unsigned short) noexcept;

    Buffer &writeInt32_LE(int) noexcept;
    Buffer &writeInt32_BE(int) noexcept;
    Buffer &writeUInt32_LE(unsigned int) noexcept;
    Buffer &writeUInt32_BE(unsigned int) noexcept;

    Buffer &writeInt64_LE(long long) noexcept;
    Buffer &writeInt64_BE(long long) noexcept;
    Buffer &writeUInt64_LE(unsigned long long) noexcept;
    Buffer &writeUInt64_BE(unsigned long long) noexcept;

    Buffer &writeFloat_LE(float) noexcept;
    Buffer &writeFloat_BE(float) noexcept;
    Buffer &writeDouble_LE(double) noexcept;
    Buffer &writeDouble_BE(double) noexcept;

    /************************** Reading ***************************/

    void setReadOffset(unsigned long long) noexcept;
    unsigned long long getReadOffset() const noexcept;
    template <class T> inline T readBytes(bool LE = true);

    bool               readBool() noexcept;
    std::string        readStr(unsigned long long len) noexcept;
    std::string        readStr() noexcept;
    char               readInt8() noexcept;
    unsigned char      readUInt8() noexcept;

    short              readInt16_LE() noexcept;
    short              readInt16_BE() noexcept;
    unsigned short     readUInt16_LE() noexcept;
    unsigned short     readUInt16_BE() noexcept;

    int                readInt32_LE() noexcept;
    int                readInt32_BE() noexcept;
    unsigned int       readUInt32_LE() noexcept;
    unsigned int       readUInt32_BE() noexcept;

    long long          readInt64_LE() noexcept;
    long long          readInt64_BE() noexcept;
    unsigned long long readUInt64_LE() noexcept;
    unsigned long long readUInt64_BE() noexcept;

    float              readFloat_LE() noexcept;
    float              readFloat_BE() noexcept;
    double             readDouble_LE() noexcept;
    double             readDouble_BE() noexcept;

    ~Buffer();
private:
    std::vector<unsigned char> buffer;
    unsigned long long readOffset = 0;
    unsigned long long writeOffset = 0;
};