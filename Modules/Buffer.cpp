#include "Buffer.hpp"
#include <iomanip> // byteStr()

/************************* WRITING *************************/

Buffer::Buffer() noexcept {
}
Buffer::Buffer(const std::string &str) noexcept :
    buffer(str.begin(), str.end()) {
}
Buffer::Buffer(const std::vector<unsigned char> &_buffer) noexcept:
    buffer(_buffer) {
}

void Buffer::setBuffer(const std::vector<unsigned char> &_buffer) noexcept {
    buffer = _buffer;
}
const std::vector<unsigned char> &Buffer::getBuffer() const noexcept {
    return buffer;
}
void Buffer::clear() noexcept {
    buffer.clear();
    readOffset = 0;
    writeOffset = 0;
}

std::string Buffer::byteStr(bool LE) const noexcept {
    std::stringstream byteStr;
    byteStr << std::hex << std::setfill('0');

    if (LE == true) {
        for (unsigned long long i = 0; i < buffer.size(); ++i)
            byteStr << std::setw(2) << (unsigned short)buffer[i] << " ";
    } else {
        unsigned long long size = buffer.size();
        for (unsigned long long i = 0; i < size; ++i)
            byteStr << std::setw(2) << (unsigned short)buffer[size - i - 1] << " ";
    }
    return byteStr.str();
}

template <class T> inline Buffer &Buffer::writeBytes(const T &val, bool LE) {
    unsigned int size = sizeof(T);

    if (LE == true) {
        for (unsigned int i = 0, mask = 0; i < size; ++i, mask += 8)
            buffer.push_back((unsigned char)(val >> mask));
    } else {
        unsigned const char *array = reinterpret_cast<unsigned const char*>(&val);
        for (unsigned int i = 0; i < size; ++i)
            buffer.push_back(array[size - i - 1]);
    }
    writeOffset += size;
    return *this;
}

unsigned long long Buffer::getWriteOffset() const noexcept {
    return writeOffset;
}

Buffer &Buffer::writeStr_UTF8(const std::string &str) noexcept {
    for (const unsigned char &s : str) 
        writeUInt8(s);
    return *this;
}
Buffer &Buffer::writeStr_UCS2(const std::string &str) noexcept {
    for (size_t i = 0; i < str.size() + (str.size() & 1); ++i)
        writeUInt8(str[i]);
    return *this;
}
Buffer &Buffer::writeStrNull_UTF8(const std::string &str) noexcept {
    writeStr_UTF8(str);
    return writeUInt8(0);
}
Buffer &Buffer::writeStrNull_UCS2(const std::string &str) noexcept {
    writeStr_UCS2(str);
    return writeUInt16_LE(0);
}

Buffer &Buffer::writeBool(bool val) noexcept {
    return writeBytes<bool>(val);
}
Buffer &Buffer::writeInt8(char val) noexcept {
    return writeBytes<char>(val);
}
Buffer &Buffer::writeUInt8(unsigned char val) noexcept {
    return writeBytes<unsigned char>(val);
}

Buffer &Buffer::writeInt16_LE(short val) noexcept {
    return writeBytes<short>(val);
}
Buffer &Buffer::writeInt16_BE(short val) noexcept {
    return writeBytes<short>(val, false);
}
Buffer &Buffer::writeUInt16_LE(unsigned short val) noexcept {
    return writeBytes<unsigned short>(val);
}
Buffer &Buffer::writeUInt16_BE(unsigned short val) noexcept {
    return writeBytes<unsigned short>(val, false);
}

Buffer &Buffer::writeInt32_LE(int val) noexcept {
    return writeBytes<int>(val);
}
Buffer &Buffer::writeInt32_BE(int val) noexcept {
    return writeBytes<int>(val, false);
}
Buffer &Buffer::writeUInt32_LE(unsigned int val) noexcept {
    return writeBytes<unsigned int>(val);
}
Buffer &Buffer::writeUInt32_BE(unsigned int val) noexcept {
    return writeBytes<unsigned int>(val, false);
}

Buffer &Buffer::writeInt64_LE(long long val) noexcept {
    return writeBytes<long long>(val);
}
Buffer &Buffer::writeInt64_BE(long long val) noexcept {
    return writeBytes<long long>(val, false);
}
Buffer &Buffer::writeUInt64_LE(unsigned long long val) noexcept {
    return writeBytes<unsigned long long>(val);
}
Buffer &Buffer::writeUInt64_BE(unsigned long long val) noexcept {
    return writeBytes<unsigned long long>(val, false);
}

Buffer &Buffer::writeFloat_LE(float val) noexcept {
    union { float fnum; unsigned inum; } u;
    u.fnum = val;
    return writeUInt32_LE(u.inum);
}
Buffer &Buffer::writeFloat_BE(float val) noexcept {
    union { float fnum; unsigned inum; } u;
    u.fnum = val;
    return writeUInt32_BE(u.inum);
}
Buffer &Buffer::writeDouble_LE(double val) noexcept {
    union { double fnum; unsigned long long inum; } u;
    u.fnum = val;
    return writeUInt64_LE(u.inum);
}
Buffer &Buffer::writeDouble_BE(double val) noexcept {
    union { double fnum; unsigned long long inum; } u;
    u.fnum = val;
    return writeUInt64_BE(u.inum);
}

/************************* READING *************************/

void Buffer::setReadOffset(unsigned long long newOffset) noexcept {
    readOffset = newOffset;
}
unsigned long long Buffer::getReadOffset() const noexcept {
    return readOffset;
}
template <class T> inline T Buffer::readBytes(bool LE) {
    T result = 0;
    unsigned int size = sizeof(T);

    // Do not overflow
    if (readOffset + size > buffer.size())
        return result;

    char *dst = (char*)&result;
    char *src = (char*)&buffer[readOffset];

    if (LE == true) {
        for (unsigned int i = 0; i < size; ++i)
            dst[i] = src[i];
    } else {
        for (unsigned int i = 0; i < size; ++i)
            dst[i] = src[size - i - 1];
    }
    readOffset += size;
    return result;
}

std::string Buffer::readStr_UTF8(unsigned long long len) noexcept {
    if (readOffset + len > buffer.size())
        len = buffer.size() - readOffset;
    std::string result(buffer.begin() + readOffset, buffer.begin() + readOffset + len);
    readOffset += len;
    return result;
}
std::string Buffer::readStr_UCS2(unsigned long long len) noexcept {
    if (readOffset + len > buffer.size())
        len = buffer.size() - readOffset;
    std::string result;
    for (unsigned long long i = 0; i < len + ((buffer.size() - readOffset) & 1); i++)
        result += readUInt8();
    return result;
}
std::string Buffer::readStrNull_UTF8() noexcept {
    unsigned long long len = readOffset;
    for (; len < buffer.size(); ++len) {
        if (+buffer[len] == 0) 
            break;
    }
    return readStr_UTF8(len);
}
std::string Buffer::readStrNull_UCS2() noexcept {
    unsigned long long len = readOffset;
    for (; len + 2 < buffer.size(); len += 2) {
        if (+(buffer[len] + buffer[len + 1]) == 0)
            break;
    }
    return readStr_UCS2(len);
}

bool Buffer::readBool() noexcept {
    return readBytes<bool>();
}
char Buffer::readInt8() noexcept {
    return readBytes<char>();
}
unsigned char Buffer::readUInt8() noexcept {
    return readBytes<unsigned char>();
}

short Buffer::readInt16_LE() noexcept {
    return readBytes<short>();
}
short Buffer::readInt16_BE() noexcept {
    return readBytes<short>(false);
}
unsigned short Buffer::readUInt16_LE() noexcept {
    return readBytes<unsigned short>();
}
unsigned short Buffer::readUInt16_BE() noexcept {
    return readBytes<unsigned short>(false);
}

int Buffer::readInt32_LE() noexcept {
    return readBytes<int>();
}
int Buffer::readInt32_BE() noexcept {
    return readBytes<int>(false);
}
unsigned int Buffer::readUInt32_LE() noexcept {
    return readBytes<unsigned int>();
}
unsigned int Buffer::readUInt32_BE() noexcept {
    return readBytes<unsigned int>(false);
}

long long Buffer::readInt64_LE() noexcept {
    return readBytes<long long>();
}
long long Buffer::readInt64_BE() noexcept {
    return readBytes<long long>(false);
}
unsigned long long Buffer::readUInt64_LE() noexcept {
    return readBytes<unsigned long long>();
}
unsigned long long Buffer::readUInt64_BE() noexcept {
    return readBytes<unsigned long long>(false);
}

float Buffer::readFloat_LE() noexcept {
    return readBytes<float>();
}
float Buffer::readFloat_BE() noexcept {
    return readBytes<float>(false);
}
double Buffer::readDouble_LE() noexcept {
    return readBytes<double>();
}
double Buffer::readDouble_BE() noexcept {
    return readBytes<double>(false);
}

Buffer::~Buffer() {
    clear();
}
