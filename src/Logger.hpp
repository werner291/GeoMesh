/*
 * (c) Copyright 2016 Werner Kroneman
 *
 * This file is part of GeoMesh.
 * 
 * GeoMesh is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * GeoMesh is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GeoMesh.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MESHNETSIM_LOGGER_H
#define MESHNETSIM_LOGGER_H

#include <string>
#include <functional>

enum LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3
};

typedef std::function<void(const LogLevel& level, const std::string& message)> LogStrategy; 

/**
 * Logger is a static-only class that handles all things related to logging.
 * Call Logger::log() to log something.
 *
 * Singletons and global variables are generally against the GeoMesh codestyle,
 * but since logging is so omnipresent, globally-accessible logging will have
 * to be a necessary evil. (Does anyone have a better idea?)
 *
 * How logging is to be handled can be set by calling setLogStrategy().
 * By default, things are just sent to standard output, but client code may
 * for example change this so that files are written to a log server.
 */
class Logger {
    static LogLevel minLevel;
public:
    static void setLogLevel(LogLevel level) {
	    minLevel = level;
    }

    static void setLogStrategy(const LogStrategy& strategy) {
        Logger::log = strategy;
    }

    static LogStrategy log;
};


#endif //MESHNETSIM_LOGGER_H

