//
// Created by Werner Kroneman on 22-01-16.
//

#include <iostream>
#include "Logger.h"

void Logger::log(LogLevel level, std::string message) {
    std::cout << std::to_string(std::time(NULL)) << ": " << message << std::endl;
}
