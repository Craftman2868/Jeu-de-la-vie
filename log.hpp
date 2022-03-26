#pragma once

#include <fstream>

#define DEBUG true


void _log(std::string message) {
#if DEBUG
    std::ofstream logFile("out.log", std::ios_base::app);

    logFile << message << std::endl;

    logFile.close();
#endif
}


void log(std::string message) {
    _log("[INFO] " + message);
}


void logError(std::string message) {
    _log("[ERROR] " + message);
}

void logWarning(std::string message) {
    _log("[WARNING] " + message);
}

void logDebug(std::string message) {
    _log("[DEBUG] " + message);
}
