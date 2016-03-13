//
// Created by Werner Kroneman on 22-01-16.
//

#include <iostream>
#include <ctime>
#include "Logger.h"

LogLevel Logger::minLevel = LogLevel::INFO;

void Logger::log(LogLevel level, std::string message) {

    if (level >= minLevel) {
        std::cout << std::to_string(std::time(NULL)) << ": " << message << std::endl;
    }
}
