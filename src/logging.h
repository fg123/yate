#ifndef LOGGING_H
#define LOGGING_H

#include <string>
#include <fstream>

class Logging {
public:
	// TODO(anyone): This is a pretty bad Logging interface...
	static std::ofstream file;
	static void info(std::string message);
};

#endif
