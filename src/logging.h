#ifndef LOGGING_H
#define LOGGING_H

#include <string>
#include <fstream>

class Log {
	std::ofstream &file;
public:
	Log(std::ofstream &file) : file(file) {}
	template<class T>
	Log &operator<<(const T &msg) {
		file << msg;
		return *this;
	}
};


class Logging {
public:
	// TODO(anyone): This is a pretty bad Logging interface...
	static std::ofstream file;
	static Log info;
	static void breadcrumb(std::string msg);
};

#endif
