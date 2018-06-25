#include "logging.h"

std::ofstream Logging::file = std::ofstream("yate.log");
Log Logging::info(Logging::file);

void Logging::breadcrumb(std::string msg) {
	file << "Breadcrumb: " << msg << std::endl;
}
