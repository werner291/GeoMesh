//
// Created by Werner Kroneman on 22-01-16.
//

#ifndef MESHNETSIM_LOGGER_H
#define MESHNETSIM_LOGGER_H

#include <string>

enum LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3
};

class Logger {
    static LogLevel minLevel;
public:
    static void setLogLevel(LogLevel level);
    static void log(LogLevel level, std::string message);
};


#endif //MESHNETSIM_LOGGER_H
