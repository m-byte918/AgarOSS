#include "Logger.hpp"

#include <sys/stat.h> // start, createDir
#include <time.h>     // dateTimeString
#ifdef _WIN32
#include <direct.h>   // createDir
#endif // _WIN32

#ifndef S_ISDIR
#define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#endif // S_ISDIR

Logger::LogLevel Logger::PRINT = { Color::White,       Color::Black, true,  true, "\n",           "", 0 };
Logger::LogLevel Logger::INFO  = { Color::BrightWhite, Color::Black, true,  true, "\n", "| [INFO]  ", 1 };
Logger::LogLevel Logger::WARN  = { Color::LightYellow, Color::Black, true,  true, "\n", "| [WARN]  ", 2 };
Logger::LogLevel Logger::ERR   = { Color::Red,         Color::Black, true,  true, "\n", "| [ERROR] ", 3 };
Logger::LogLevel Logger::FATAL = { Color::LightRed,    Color::Black, true,  true, "\n", "| [FATAL] ", 4 };
Logger::LogLevel Logger::DEBUG = { Color::LightGreen,  Color::Black, false, true, "\n", "| [DEBUG] ", 5 };

Logger::Logger() {
    start();
}

Logger::Logger(const std::string &logName) {
    LOG_NAME = logName;
    start();
}

bool Logger::createDir(const char *dir) {
    struct stat info;

    if (stat(dir, &info) != 0 || !S_ISDIR(info.st_mode)) {
        #ifdef _WIN32
            return _mkdir(dir) > -1;
        #else
            return mkdir(dir, 0733) > -1;
        #endif // _WIN32
    }
    return false;
}

std::string Logger::dateTimeString(bool returnTimeOnly) {
    char      buf[80];
    time_t    now = time(0);
    #pragma warning(push)
    #pragma warning(disable: 4996)
    struct tm tstruct = *localtime(&now);
    #pragma warning(pop)

    if (returnTimeOnly)
        strftime(buf, sizeof(buf), "%H;%M;%S %p", &tstruct);
    else
        strftime(buf, sizeof(buf), "%Y-%m-%d %H;%M;%S %p", &tstruct);
    return buf;
}

void Logger::resetColors() noexcept {
    #ifdef _WIN32
        setConsoleInfo();
        SetConsoleTextAttribute(hOut, defaultColorAttribs);
    #else
        std::cout << "\e[0m\n";
        lastFgColor = "39";
        lastConsoleColor = "49";
    #endif // _WIN32
}

void Logger::clearConsole() noexcept {
    #ifdef _WIN32
        setConsoleInfo();
        GetConsoleScreenBufferInfo(hOut, &csbi);
        DWORD cellCount = csbi.dwSize.X * csbi.dwSize.Y;
        DWORD count;
        COORD home = { 0, 0 };
    
        FillConsoleOutputCharacter(hOut, ' ', cellCount, home, &count);
        FillConsoleOutputAttribute(hOut, csbi.wAttributes, cellCount, home, &count);
        SetConsoleCursorPosition(hOut, home);
    #else
        if (lastConsoleColor != "49")
            std::cout << "\e[0;" << lastFgColor << ";" << lastConsoleColor << "m";
        std::cout << "\e[2J\e[1;1H\n";
    #endif // _WIN32
}

void Logger::setTextColor(const Color &foreground, const Color &background) noexcept {
    if (background < 0 || foreground < 0 || background > 15 || foreground > 15) {
        std::cerr << "Colors only range from 0 to 15.\n";
        return;
    }
    #ifdef _WIN32
        setConsoleInfo();
        SetConsoleTextAttribute(hOut, (WORD)(foreground + (background * 16)));
    #else
        lastFgColor = fc[foreground];
        std::cout << "\e[0;" << lastFgColor << ";" << bc[background] << "m";
    #endif // _WIN32
}

void Logger::setConsoleColor(const Color &color) noexcept {
    if (color < 0 || color > 15) {
        std::cerr << "Colors only range from 0 to 15.\n";
        return;
    }
    #ifdef _WIN32
        setConsoleInfo();
        SetConsoleTextAttribute(hOut, (WORD)(color << 4));
    #else
        lastConsoleColor = bc[color];
        std::cout << "\e[0;" << lastFgColor << ";" << lastConsoleColor << "m";
    #endif // _WIN32
    clearConsole();
}

void Logger::setSeverity(int level) noexcept {
    maxSeverity = level;
}

void Logger::setFileSeverity(int level) noexcept {
    maxFileSeverity = level;
}

int &Logger::getSeverity() noexcept {
    return maxSeverity;
}

int &Logger::getFileSeverity() noexcept {
    return maxFileSeverity;
}

void Logger::start() {
    if (file.is_open()) return;

    try {
        struct stat info;
        const std::string timeStr        = dateTimeString();
        const std::string fileName       = LOG_FLDR + "/" + LOG_NAME + ".log";
        const std::string backupFileName = LOG_BACKUP_FLDR + "/" + LOG_NAME + "-" + timeStr + ".log";

        createDir(LOG_FLDR.c_str());

        if (stat(fileName.c_str(), &info) == 0) {
            // Backup previous log
            createDir(LOG_BACKUP_FLDR.c_str());
            std::rename(fileName.c_str(), backupFileName.c_str());
        }
        file.open(fileName, std::ofstream::out);
        if (file.fail()) throw;

        file << "=== Started " + timeStr + " ===\n";
    } catch (...) {
        file.close();
        std::cerr << "Error starting logger\n";
    }
}

void Logger::end() {
    if (!file.is_open()) return;
    file << "=== Shutdown " + dateTimeString() + " ===\n";
    file.close();
}

Logger::~Logger() {
    end();
}

#ifdef _WIN32
const HANDLE Logger::hOut = GetStdHandle(STD_OUTPUT_HANDLE);
WORD Logger::defaultColorAttribs;
CONSOLE_SCREEN_BUFFER_INFO Logger::csbi;

void Logger::setConsoleInfo() noexcept {
    GetConsoleScreenBufferInfo(hOut, &csbi);
    if (!csbi.dwMaximumWindowSize.X && !csbi.dwMaximumWindowSize.Y)
        defaultColorAttribs = csbi.wAttributes;
}
#else
const char *Logger::lastFgColor = "39";
const char *Logger::lastConsoleColor = "49";
const char *Logger::fc[16] = { "30","34","32","36","31","35","33","37", "90", "94", "92", "96", "91", "95", "93", "97" };
const char *Logger::bc[16] = { "40","44","42","46","41","45","43","47","100","104","102","106","101","105","103","107" };
#endif

std::string Logger::LOG_NAME = "MainLog";
std::string Logger::LOG_FLDR = "./logs";
std::string Logger::LOG_BACKUP_FLDR = "./logs/LogBackups";

std::ofstream Logger::file;
int Logger::maxSeverity = 5;
int Logger::maxFileSeverity = 5;