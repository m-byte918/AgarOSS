#pragma once

#include <iostream>  // writeTo, setTextColor, setConsoleColor, start
#include <string>    // Folder and file names
#include <fstream>   // Log recording
#ifdef _WIN32
#include <Windows.h> // Colors for Windows
#endif // _WIN32

class Logger {
public:
    /////////////////////////////////////////
    /// \enum Logger::Color
    /// \brief Standard console colors
    ////////////////////////////////////////
    enum Color {
        Black, Blue, Green, Aqua, Red, Purple, Yellow, White, Gray, LightBlue, 
        LightGreen, LightAqua, LightRed, LightPurple, LightYellow, BrightWhite
    };

    /////////////////////////////////////////
    /// \struct Logger::LogLevel
    /// \brief Properties for each log level
    ////////////////////////////////////////
    static struct LogLevel {
        Color      fgColor; // Text foreground color
        Color      bgColor; // Text background color
        bool    enumerable; // If level is affected by 'max severity' checks before being written
        bool      writable; // If level is able to be written to log file
        const char *suffix; // Suffix of log-level added after string
        const char *prefix; // Prefix of log-level added before string
        int       severity; // Severity of log-level
    } PRINT, INFO, WARN, ERR, FATAL, DEBUG;

    ////////////////////////////////
    /// \brief Constructs a logger
    /// \note Calls Logger::start()
    /// \see Logger::start()
    ///////////////////////////////
    Logger();

    //////////////////////////////////////////////////////
    /// \brief Constructs a logger and sets it's log name 
    /// \note Calls Logger::start()
    /// \param logName Name of log to be set
    /// \see Logger::start(), Logger::LOG_NAME
    /////////////////////////////////////////////////////
    Logger(const std::string &logName);

    ///////////////////////////////////////////////////////////////
    /// \brief Creates a directory
    /// \param dir Name and/or path of the directory to be created
    /// \returns True if the directory was created successfully
    //////////////////////////////////////////////////////////////
    static bool createDir(const char *dir);

    ////////////////////////////////////////////////////////////////////
    /// \brief Creates a string of the current date and/or time
    /// \warning Function's use of localtime() is not thread safe
    /// \param returnTimeOnly Return a string of the current time only?
    /// \returns String containing the current date and/or time
    ///////////////////////////////////////////////////////////////////
    static std::string dateTimeString(bool returnTimeOnly = false);

    /////////////////////////////////////
    /// \brief Resets all console colors
    ////////////////////////////////////
    static void resetColors() noexcept;

    /////////////////////////////////
    /// \brief Clears console output
    ////////////////////////////////
    static void clearConsole() noexcept;

    ////////////////////////////////////////////////////////////////
    /// \brief Sets the foreground and background color of text.
    /// \note This does not affect log level colors
    /// \param foreground Foreground color of text
    /// \param background Background color of text (default: black)
    /// \see Logger::Color
    ///////////////////////////////////////////////////////////////
    static void setTextColor(const Color &foreground, const Color &background = Color::Black) noexcept;

    ///////////////////////////////////////////////////////////
    /// \brief Sets background color of the entire console.
    /// \note This does not affect log level background colors
    /// \param color Color of console to be set
    /// \see Logger::Color
    //////////////////////////////////////////////////////////
    static void setConsoleColor(const Color &color) noexcept;

    //////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Writes message with log-level properties to file and/or console
    /// \note Messages will not be written to log if start() was not called previously
    /// \param lvl LogLevel to have its properties added onto message
    /// \param msg Message to be written
    /// \param toLog Write message to log file?
    /// \param toConsole Write message to console?
    /// \see Logger::PRINT, Logger::INFO, Logger::WARN, Logger::ERR, Logger::FATAL, Logger::DEBUG;
    //////////////////////////////////////////////////////////////////////////////////////////////
    template <class T>
    static void write(const LogLevel *lvl, const T &msg, bool toLog, bool toConsole) {
        std::ios_base::sync_with_stdio(false);
        // Write message to file
        if (toLog && file.is_open() && lvl->writable && (!lvl->enumerable || lvl->severity < maxFileSeverity)) {
            try {
                file << "[" + dateTimeString(true) + "] " << lvl->prefix << msg << lvl->suffix;
            } catch (...) {
                file.close();
                std::cerr << "Error writing to log.\n";
            }
        }
        // Write message to console
        if (toConsole && (!lvl->enumerable || lvl->severity < maxSeverity)) {
            // Print colored string
            setTextColor(lvl->fgColor, lvl->bgColor);
            std::cout << lvl->prefix << msg << lvl->suffix;
            // Reset colors
            #ifdef _WIN32
                SetConsoleTextAttribute(hOut, csbi.wAttributes);
            #else
                std::cout << "\e[0m\n";
            #endif // _WIN32
        }
    }

    /////////////////////////////////////////////////////////////////////////
    /// \brief Writes message + PRINT LogLevel properties to log and console
    /// \param msg Message to write
    /// \see Logger::PRINT
    ////////////////////////////////////////////////////////////////////////
    template <class T> static void print(const T &msg) { write(&PRINT, msg, true, true); }

    ////////////////////////////////////////////////////////////////////////
    /// \brief Writes message + INFO LogLevel properties to log and console
    /// \param msg Message to write
    /// \see Logger::INFO
    ///////////////////////////////////////////////////////////////////////
    template <class T> static void info(const T &msg) { write(&INFO, msg, true, true); }

    ////////////////////////////////////////////////////////////////////////
    /// \brief Writes message + WARN LogLevel properties to log and console
    /// \param msg Message to write
    /// \see Logger::WARN
    ///////////////////////////////////////////////////////////////////////
    template <class T> static void warn(const T &msg) { write(&WARN, msg, true, true); }

    ///////////////////////////////////////////////////////////////////////
    /// \brief Writes message + ERR LogLevel properties to log and console
    /// \param msg Message to write
    /// \see Logger::ERR
    //////////////////////////////////////////////////////////////////////
    template <class T> static void error(const T &msg) { write(&ERR, msg, true, true); }

    /////////////////////////////////////////////////////////////////////////
    /// \brief Writes message + FATAL LogLevel properties to log and console
    /// \param msg Message to write
    /// \see Logger::FATAL
    ////////////////////////////////////////////////////////////////////////
    template <class T> static void fatal(const T &msg) { write(&FATAL, msg, true, true); }

    /////////////////////////////////////////////////////////////////////////
    /// \brief Writes message + DEBUG LogLevel properties to log and console
    /// \param msg Message to write
    /// \see Logger::DEBUG
    ////////////////////////////////////////////////////////////////////////
    template <class T> static void debug(const T &msg) { write(&DEBUG, msg, true, true); }

    /////////////////////////////////////////////////////////////
    /// \brief Writes message + PRINT LogLevel properties to log
    /// \param msg Message to write
    /// \see Logger::PRINT
    ////////////////////////////////////////////////////////////
    template <class T> static void logMessage(const T &msg) { write(&PRINT, msg, true, false); }

    ///////////////////////////////////////////////////////////
    /// \brief Writes message + ERR LogLevel properties to log
    /// \param msg Message to write
    /// \see Logger::ERR
    //////////////////////////////////////////////////////////
    template <class T> static void logError(const T &msg) { write(&ERR, msg, true, false); }

    /////////////////////////////////////////////////////////////
    /// \brief Writes message + DEBUG LogLevel properties to log
    /// \param msg Message to write
    /// \see Logger::DEBUG
    ////////////////////////////////////////////////////////////
    template <class T> static void logDebug(const T &msg) { write(&DEBUG, msg, true, false); }

    ////////////////////////////////////////////////////////////
    /// \brief Sets maximum LogLevel severity for console write
    /// \param level New maximum severity
    /// \see Logger::maxSeverity
    ///////////////////////////////////////////////////////////
    static void setSeverity(int level) noexcept;

    /////////////////////////////////////////////////////////
    /// \brief Sets maximum LogLevel severity for log output
    /// \param level New maximum severity
    /// \see Logger::maxFileSeverity
    ////////////////////////////////////////////////////////
    static void setFileSeverity(int level) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Gets maximum LogLevel severity for console write
    /// \returns Maximum LogLevel severity for console write
    /// \see Logger::maxSeverity
    ///////////////////////////////////////////////////////////
    static int &getSeverity() noexcept;

    /////////////////////////////////////////////////////////
    /// \brief Gets maximum LogLevel severity for log output
    /// \returns Maximum LogLevel severity for log output
    /// \see Logger::maxFileSeverity
    ////////////////////////////////////////////////////////
    static int &getFileSeverity() noexcept;

    ///////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Opens log file stream for recording logs
    /// \note Logging is still possible without calling start(), but logs will not be recorded
    /// \see Logger::file
    //////////////////////////////////////////////////////////////////////////////////////////
    static void start();

    //////////////////////////////////////////////////////////////////
    /// \brief Closes log file stream, saves and stops recording logs
    /// \see Logger::file
    /////////////////////////////////////////////////////////////////
    static void end();

    ///////////////////////////////
    /// \brief Destroys the logger
    /// \note Calls Logger::end()
    /// \see Logger::end()
    //////////////////////////////
    ~Logger();

private:
    // OS specific variables
    #ifdef _WIN32
        const static HANDLE hOut;
        static WORD defaultColorAttribs;
        static CONSOLE_SCREEN_BUFFER_INFO csbi;
    
        // Remember original attributes
        static void setConsoleInfo() noexcept;
    #else
        static const char *lastFgColor;
        static const char *lastConsoleColor;
        static const char *fc[16];
        static const char *bc[16];
    #endif // _WIN32

    static std::string LOG_NAME;        // Name of log file
    static std::string LOG_FLDR;        // Name of log folder
    static std::string LOG_BACKUP_FLDR; // Name of log backups folder

    static std::ofstream file;  // Log file
    static int maxSeverity;     // Maximum log-level severity for console write
    static int maxFileSeverity; // Maximum log-level severity for file write
};