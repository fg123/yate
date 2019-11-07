#include "logging.h"

#include <iostream>

_Log Logging::info;
_Log Logging::error;
std::vector<std::ostream*> Logging::streams;
std::vector<LogListener*> Logging::listeners;
std::queue<std::string> Logging::buffer;

void Logging::init(std::string path) {
  Logging::streams.push_back(new std::ofstream(path));
  if (path.empty()) {
    Logging::streams[0]->setstate(std::ios_base::badbit);
  }
}

void Logging::cleanup() {
  for (auto stream : Logging::streams) {
    delete stream;
  }
}

void Logging::breadcrumb(std::string msg) {
  Logging::info << "Breadcrumb: " << msg << std::endl;
}

