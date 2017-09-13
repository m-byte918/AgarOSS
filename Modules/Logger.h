#pragma once

#include <iostream>
#include <string>
#include <sys/stat.h> // start(), createDir()
#include <time.h>     // dateTimeString()
#include <fstream>    // Log recording

#ifdef _WIN32
    #include <windows.h>
    const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    WORD og_colors;
#endif // _WIN32

namespace logger {

    // Log folder/file names and locations
    const std::string LOG_FLDR        = "./logs";
    const std::string LOG_FILE_NAME   = "MainLog";
    const std::string LOG_BACKUP_FLDR = LOG_FLDR + "/LogBackups";

    uint8_t max_severity      = 5; // Maximum log-level severity for console write
    uint8_t max_file_severity = 5; // Maximum log-level severity for file write

    // Log-Level specific properties.
    // Can be accessed externally by logger::LEVEL.property;
    struct LogLevel_NONE {
        uint8_t colorAttrib = 0 << 4 | 7; // Color format: BACKGROUND << 4 | FOREGROUND (background for Windows only)
        uint8_t severity    = 0;          // Severity of log-level
        bool enumerable     = true;       // Level is affected by 'max severity' checks before being written
        bool writable       = true;       // Level is able to be written to the console or log files
        std::string prefix  = "";         // Prefix of log-level added before string
    } PRINT;

    struct LogLevel_INFO {
        uint8_t colorAttrib = 0 << 4 | 15;
        uint8_t severity    = 1;
        bool enumerable     = true;
        bool writable       = true;
        std::string prefix  = "| [INFO] ";
    } INFO;

    struct LogLevel_WARN {
        uint8_t colorAttrib = 0 << 4 | 14;
        uint8_t severity    = 2;
        bool enumerable     = true;
        bool writable       = true;
        std::string prefix  = "| [WARN] "; 
    } WARN;

    struct LogLevel_ERROR {
        uint8_t colorAttrib = 0 << 4 | 4;
        uint8_t severity    = 3;
        bool enumerable     = true;
        bool writable       = true;
        std::string prefix  = "| [ERROR] ";
    } ERR;

    struct LogLevel_FATAL {
        uint8_t colorAttrib = 0 << 4 | 12;
        uint8_t severity    = 4;
        bool enumerable     = true;
        bool writable       = true;
        std::string prefix  = "| [FATAL] ";
    } FATAL;

    struct LogLevel_DEBUG {
        uint8_t colorAttrib = 0 << 4 | 10;
        uint8_t severity    = 5;
        bool enumerable     = false;
        bool writable       = true;
        std::string prefix  = "| [DEBUG] ";
    } DEBUG;

    std::ofstream file;

    // Syntax: logger::createDir("dirName");
    // Creates directory if it does not already exist
    void createDir(const char *name) {
        struct stat info;

        if (stat(name, &info) != 0 || !S_ISDIR(info.st_mode)) {
            #if defined(_WIN32)
                mkdir(name); // for Windows
            #else
                mkdir(name, 0733); // for non-Windows
            #endif
        }
    }

    // Syntax: logger::dateTimeString(0 | 1);
    // Set parameter to 1 to display the time only.
    // Returns string of the current date/time
    std::string dateTimeString(const int &displayTimeOnly = 0) {
        char      buf[80];
        time_t    now = time(0);
        struct tm tstruct = *localtime(&now);

        if (displayTimeOnly == 1)
            strftime(buf, sizeof(buf), "%H;%M;%S %p", &tstruct);
        else if (displayTimeOnly == 0)
            strftime(buf, sizeof(buf), "%Y-%m-%d %H;%M;%S %p", &tstruct);
        return buf;
    }

    // Ignore this :p
    // Miscellaneous function for color-setting
    int canSet = 1;
    int misc(const int &c = -1, const int &from_out = 0) {
        #if defined(_WIN32)
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            GetConsoleScreenBufferInfo(hOut, &csbi);
            if (!og_colors) og_colors = csbi.wAttributes;
        #endif
        if (from_out >= 0) canSet = 0;
        if (from_out >= 0 && c == -1)
            canSet = 1;
        return canSet == 1 || from_out >= 0 ? 1 : 0;
    }

    // Syntax: logger::setColor(0-15);
    // Set parameter to -1 to reset all console colors.
    // Sets console's foreground (text) color.
    void setColor(int c = -1, const int &from_out = 0) {
        if (misc(c, from_out) == 0) return;
        if (c > 15 || c < -1) {
            std::cout << "Foreground colors can only be -1 through 15.\n";
            c >>= 4;
        }
        
        #if defined(_WIN32)
            if (c != -1) SetConsoleTextAttribute(hOut, c);
            else SetConsoleTextAttribute(hOut, og_colors);
        #else
            switch (c) {
                case 0:  std::cout << "\e[22;30m"; break; // Black
                case 1:  std::cout << "\e[22;34m"; break; // Blue
                case 2:  std::cout << "\e[22;32m"; break; // Green
                case 3:  std::cout << "\e[22;36m"; break; // Aqua
                case 4:  std::cout << "\e[22;31m"; break; // Red
                case 5:  std::cout << "\e[22;35m"; break; // Purple
                case 6:  std::cout << "\e[22;33m"; break; // Yellow
                case 7:  std::cout << "\e[22;37m"; break; // White
                case 8:  std::cout << "\e[01;30m"; break; // Gray
                case 9:  std::cout << "\e[01;34m"; break; // Light Blue
                case 10: std::cout << "\e[01;32m"; break; // Light Green
                case 11: std::cout << "\e[01;36m"; break; // Light Aqua
                case 12: std::cout << "\e[01;31m"; break; // Light Red
                case 13: std::cout << "\e[01;35m"; break; // Light Purple
                case 14: std::cout << "\e[01;33m"; break; // Light Yellow
                case 15: std::cout << "\e[01;37m"; break; // Bright White
                default: std::cout << "\e[0m"; break;     // Reset colors
            }
        #endif
    }

    // Called from logger functions below
    // Syntax: logger::writeTo(&LEVEL, message, Log, Con);
    // Parameters Log and Con accept either 0 or 1.
    // Writes message + log-level properties to file or console
    void writeTo(const auto *lvl, const auto &msg, const int &Log, const int &Con) {
        std::ios_base::sync_with_stdio(false);
        // Write message to file
        if (Log == 1 && file.is_open() && lvl->writable
            && (!lvl->enumerable || lvl->severity < max_file_severity)) {
            std::string newPrfx = lvl->prefix + "["+dateTimeString(1)+"] ";

            try {
                if (file.fail()) throw;
                file << newPrfx << msg << "\n"; 
            } catch (...) {
                file.close();
                writeTo(&ERR, "Error writing to log!", 0, 1);
            }
        }
        // Write message to console
        if (Con == 1 && (!lvl->enumerable || lvl->severity < max_severity)) {
            setColor(lvl->colorAttrib, -1); // Set string color
            std::cout << lvl->prefix << msg << "\n";
            setColor(-1, -1); // Reset color (-1)
        }
    }

    // Example syntax: logger::print(message)
    // Outputs message to console and log file
    void print(const auto &msg) { writeTo(&PRINT, msg, 1, 1); }
    void info (const auto &msg) { writeTo(&INFO, msg, 1, 1);  }
    void warn (const auto &msg) { writeTo(&WARN, msg, 1, 1);  }
    void error(const auto &msg) { writeTo(&ERR, msg, 1, 1);   }
    void fatal(const auto &msg) { writeTo(&FATAL, msg, 1, 1); }
    void debug(const auto &msg) { writeTo(&DEBUG, msg, 1, 1); }

    // Example syntax: logger::write(message)
    // Outputs message to log file only
    void write(const auto &msg) { writeTo(&PRINT, msg, 1, 0); }
    void writeError(const auto &msg) { writeTo(&ERR, msg, 1, 0);   }
    void writeDebug(const auto &msg) { writeTo(&DEBUG, msg, 1, 0); }

    // Example syntax: logger::setSeverity(int);
    // Sets max log-level severity for console or log file output
    void setSeverity(const int &level) { max_severity = level; }
    void setFileSeverity(const int &level) { max_file_severity = level; }

    // Example syntax: logger::getSeverity(int);
    // Gets max log-level severity for console or log file output
    const uint8_t &getSeverity() { return max_severity; }
    const uint8_t &getFileSeverity() { return max_file_severity; }

    // Syntax: logger::clear();
    // Clears logger console output
    void clear() {
        #if defined(_WIN32)
            SetConsoleCursorPosition(hOut, {0, 0});
            for (int i=0;i<=100;++i)
                std::cout << std::string(100, ' ');
            SetConsoleCursorPosition(hOut, {0, 0});
        #else
            std::cout << "\e[2J\e[3J\e[H";
        #endif
    }

    // Syntax: logger::setBackgroundColor(0-7);
    // Set parameter to -1 to reset all console colors.
    // Sets console's background color.
    void setBackgroundColor(int c = -1, const int &from_out = 1) {
        if (misc(c, from_out) == 0) return;
        if ((c > 7 || c < -1) && from_out == 1) {
            warn("Background colors can only be -1 through 7.");
            c >>= 4;
        }
        #if defined(_WIN32)
            if (c != -1) SetConsoleTextAttribute(hOut, c << 4);
            else SetConsoleTextAttribute(hOut, og_colors);
        #else
            switch (c) {
                case 0:  std::cout << "\e[40m"; break; // Black
                case 1:  std::cout << "\e[41m"; break; // Red
                case 2:  std::cout << "\e[42m"; break; // Green
                case 3:  std::cout << "\e[43m"; break; // Yellow
                case 4:  std::cout << "\e[44m"; break; // Blue
                case 5:  std::cout << "\e[45m"; break; // Purple
                case 6:  std::cout << "\e[46m"; break; // Aqua
                case 7:  std::cout << "\e[47m"; break; // White
                default: std::cout << "\e[0m";  break; // Reset colors
            }
        #endif
        clear(); // Clear output to apply background changes
    }

    // Syntax: logger::start();
    // Opens log file stream for recording logs.
    // Logging is still possible without calling start, but logs will not be recorded.
    void start() {
        if (file.is_open())
            return;

        try {
            struct stat info;
            const std::string time_str    = dateTimeString();
            const std::string FILE_NAME   = LOG_FLDR+"/"+LOG_FILE_NAME+".log";
            const std::string BACKUP_NAME = LOG_BACKUP_FLDR+"/"+LOG_FILE_NAME+"-"+time_str+".log";

            createDir(LOG_FLDR.c_str());
            if (stat(FILE_NAME.c_str(), &info) == 0) {
                // Backup previous log
                createDir(LOG_BACKUP_FLDR.c_str());
                std::rename(FILE_NAME.c_str(), BACKUP_NAME.c_str());
            }

            file.open(FILE_NAME);
            if (file.fail()) throw;
            file << "=== Started " + dateTimeString() + " ===\n";
        } catch (...) {
            file.close();
            writeTo(&ERR, "Error starting logger", 0, 1);
        }
    }

    // Syntax: logger::end();
    // Closes log file stream, saves and stops recording logs.
    void end() {
        if (!file.is_open()) return;
        file << "=== Shutdown " + dateTimeString() + " ===\n";
        file.close();
    }

}; // namespace logger
