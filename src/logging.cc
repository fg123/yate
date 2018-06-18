#include "logging.h"

std::ofstream Logging::file = std::ofstream("yate.log", std::ios::app);

void Logging::info(std::string message) {
	file << message << std::endl;
}
