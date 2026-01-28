// logger.h
#pragma once

#include <Arduino.h>

#define LOG_LEVEL_TRACE 0
#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_WARN 3
#define LOG_LEVEL_ERROR 4
#define LOG_LEVEL_FATAL 5
#define LOG_LEVEL_OFF 6

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_DEBUG
#endif

class Logger {
    private:
    bool m_show_level;
    bool m_show_location;
    Stream* m_output;

    Logger() : m_show_level(true),
               m_show_location(true),
               m_output(&Serial) {}

    Logger(const Logger&)            = delete;
    Logger& operator=(const Logger&) = delete;

    void print_formatted_impl(const char* format) {
        while (*format) {
            m_output->print(*format++);
        }
    }

    template <typename T, typename... Args>
    void print_formatted_impl(const char* format, T value, Args... args) {
        while (*format) {
            if (*format == '{' && *(format + 1) == '}') {
                m_output->print(value);
                print_formatted_impl(format + 2, args...);
                return;
            }
            m_output->print(*format++);
        }
    }

    template <typename... Args>
    void printFormattedFlash(const __FlashStringHelper* format, Args... args) {
        PGM_P p = reinterpret_cast<PGM_P>(format);
        char buffer[256];
        strncpy_P(buffer, p, sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';
        print_formatted_impl(buffer, args...);
    }

    void print_header(const __FlashStringHelper* level, const __FlashStringHelper* file, int line) {
        if (m_show_level) {
            m_output->print('[');
            m_output->print(level);
            m_output->print(F("] "));
        }
        if (m_show_location) {
            m_output->print('[');
            m_output->print(file);
            m_output->print(':');
            m_output->print(line);
            m_output->print(F("] "));
        }
    }

    public:
    static Logger& instance() {
        static Logger logger;
        return logger;
    }

    void begin(unsigned long baud_rate) { Serial.begin(baud_rate); }
    void setShowLevel(bool show) { m_show_level = show; }
    void setShowLocation(bool show) { m_show_location = show; }
    void setOutput(Stream* output) { m_output = output; }

    // TRACE
    template <typename... Args>
    void trace(const __FlashStringHelper* file, int line, const __FlashStringHelper* format, Args... args) {
        if (!m_output) return;
        print_header(F("TRACE"), file, line);
        printFormattedFlash(format, args...);
        m_output->println();
    }

    // DEBUG
    template <typename... Args>
    void debug(const __FlashStringHelper* file, int line, const __FlashStringHelper* format, Args... args) {
        if (!m_output) return;
        print_header(F("DEBUG"), file, line);
        printFormattedFlash(format, args...);
        m_output->println();
    }

    // INFO
    template <typename... Args>
    void info(const __FlashStringHelper* file, int line, const __FlashStringHelper* format, Args... args) {
        if (!m_output) return;
        print_header(F("INFO "), file, line);
        printFormattedFlash(format, args...);
        m_output->println();
    }

    // WARN
    template <typename... Args>
    void warn(const __FlashStringHelper* file, int line, const __FlashStringHelper* format, Args... args) {
        if (!m_output) return;
        print_header(F("WARN "), file, line);
        printFormattedFlash(format, args...);
        m_output->println();
    }

    // ERROR
    template <typename... Args>
    void error(const __FlashStringHelper* file, int line, const __FlashStringHelper* format, Args... args) {
        if (!m_output) return;
        print_header(F("ERROR"), file, line);
        printFormattedFlash(format, args...);
        m_output->println();
    }

    // FATAL
    template <typename... Args>
    void fatal(const __FlashStringHelper* file, int line, const __FlashStringHelper* format, Args... args) {
        if (!m_output) return;
        print_header(F("FATAL"), file, line);
        printFormattedFlash(format, args...);
        m_output->println();
    }
};

inline Logger& log() {
    return Logger::instance();
}

#ifdef ENABLE_LOGGING

#define LOG_BEGIN(val) log().begin(val);
#define LOG_SETSHOWLEVEL(val) log().setShowLevel(val)
#define LOG_SETSHOWLOCATION(val) log().setShowLocation(val)

#if LOG_LEVEL <= LOG_LEVEL_TRACE
#define LOG_TRACE(format, ...) log().trace(F(__FILE__), __LINE__, F(format), ##__VA_ARGS__)
#else
#define LOG_TRACE(format, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LOG_DEBUG(format, ...) log().debug(F(__FILE__), __LINE__, F(format), ##__VA_ARGS__)
#else
#define LOG_DEBUG(format, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOG_INFO(format, ...) log().info(F(__FILE__), __LINE__, F(format), ##__VA_ARGS__)
#else
#define LOG_INFO(format, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_WARN
#define LOG_WARN(format, ...) log().warn(F(__FILE__), __LINE__, F(format), ##__VA_ARGS__)
#else
#define LOG_WARN(format, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define LOG_ERROR(format, ...) log().error(F(__FILE__), __LINE__, F(format), ##__VA_ARGS__)
#else
#define LOG_ERROR(format, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_FATAL
#define LOG_FATAL(format, ...) log().fatal(F(__FILE__), __LINE__, F(format), ##__VA_ARGS__)
#else
#define LOG_FATAL(format, ...)
#endif

#else

#define LOG_SHOWLEVEL(val)
#define LOG_SHOWLOCATION(val)
#define LOG_TRACE(format, ...)
#define LOG_DEBUG(format, ...)
#define LOG_INFO(format, ...)
#define LOG_WARN(format, ...)
#define LOG_ERROR(format, ...)
#define LOG_FATAL(format, ...)

#endif