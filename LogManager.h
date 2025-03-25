#pragma once

#include "Globals.h"

class LogManager {
public:
	void InitializeLog();
	static void pPrint(bool writeToFile, bool printConsole, const char* format, ...);
	void CloseLog();
};