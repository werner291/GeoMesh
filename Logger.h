//
// Created by Werner Kroneman on 22-01-16.
//

#ifndef MESHNETSIM_LOGGER_H
#define MESHNETSIM_LOGGER_H

#include <string>

enum LogLevel {
    DEBUG,
    WARN,
    ERROR
};

class Logger {
public:
    static void log(LogLevel level, std::string message);
};


#endif //MESHNETSIM_LOGGER_H
