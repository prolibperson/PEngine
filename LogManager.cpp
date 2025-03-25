#include "LogManager.h"

std::ofstream logFile;

void LogManager::InitializeLog() {
    std::time_t now = std::time(nullptr);
    std::tm localTime;
    localtime_s(&localTime, &now);

    std::ostringstream filename;
    filename << "Log/PLog-" << std::put_time(&localTime, "%Y-%m-%d-%H-%M-%S") << ".txt";

    logFile.open(filename.str());
    if (!logFile.is_open()) {
        std::cerr << "Error opening " << filename.str() << std::endl;
    }
}

void LogManager::pPrint(bool writeToFile, bool printConsole, const char* format, ...) {
    va_list args;
    va_start(args, format);

    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);

    if (printConsole) {
        std::cout << buffer;
    }

    if (logFile.is_open() && writeToFile) {
        logFile << buffer;
    }

    va_end(args);
}

void LogManager::CloseLog() {
    if (logFile.is_open()) {
        logFile.close();
    }
}