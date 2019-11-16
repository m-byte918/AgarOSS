#include "Utils.hpp"
#include <sstream> // Color::toString()
#include "../Game/Game.hpp"

namespace utils {

Color::Color() :
    Color(0, 0, 0) {
}
Color::Color(const json::value_type &j) {
    r = j[0];
    g = j[1];
    b = j[2];
}
Color::Color(const json &j0, const json &j1, const json &j2) {
    r = j0;
    g = j1;
    b = j2;
}
Color::Color(unsigned char _r, unsigned char _g, unsigned char _b) :
    r(_r),
    g(_g),
    b(_b) {
}

std::string Color::toString() {
    std::stringstream ss;
    ss << "{ " << +r << ", " << +g << ", " << +b << " }";
    return ss.str();
}

bool Color::operator==(const Color &other) {
    return r == other.r && g == other.g && b == other.b;
}
bool Color::operator!=(const Color &other) {
    return r != other.r || g != other.g || b != other.b;
}

// Returns flag from a json list
unsigned char getFlagFrom(const json::value_type &j) {
    unsigned char flag = 0x00;
    for (const std::string &s : j) {
        if (s == "food") flag |= food;
        else if (s == "viruses") flag |= viruses;
        else if (s == "ejected") flag |= ejected;
        else if (s == "mothercells") flag |= mothercells;
        else if (s == "playercells") flag |= playercells;
        else flag |= nothing;
    }
    return flag;
}

// Returns a vector of strings from provided string
std::vector<std::string> splitStr(const std::string &str, char delimiter) {
    std::vector<std::string> ret;
    std::string::const_iterator i = str.begin();

    while (i != str.end()) {
        // ignore leading blanks
        i = std::find_if(i, str.end(), [&](char c) { return c != delimiter; });
        // find end of next word
        std::string::const_iterator j = std::find_if(i, str.end(),
            [&delimiter](char c) { return c == delimiter; });
        // copy the characters in [i, j)
        if (i != str.end())
            ret.push_back(std::string(i, j));
        i = j;
    }
    return ret;
}

// Check if string starts with '-', and ONLY contains one or more of the characters provided
bool checkFlagStr(std::string str, const std::string &flagChars) {
    if (str.front() != '-') return false;
    str.erase(str.begin());

    if (str.find_first_not_of(flagChars.c_str()) != std::string::npos)
        return false;

    for (char c : str) {
        if (std::count(str.begin(), str.end(), c) > 1)
            return false;
    }
    return true;
}

extern Color randomColor() noexcept {
    unsigned char RGB[3] = {
        255,
        7,
        (unsigned char)rand(0, 256)
    };
    static std::random_device rd;
    std::shuffle(&RGB[0], &RGB[3], rd);
    return { RGB[0], RGB[2], RGB[1] };
}

extern Vec2 randomPosition() noexcept {
    double halfWidth = cfg::game_mapWidth * 0.5f;
    double halfHeight = cfg::game_mapHeight * 0.5f;
    return {
        rand(-halfWidth,  halfWidth),
        rand(-halfHeight, halfHeight)
    };
}

} // namespace utils