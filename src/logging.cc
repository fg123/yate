#include "logging.h"

#include <iostream>

std::ostream *Logging::stream = nullptr;
Log Logging::info;
Log Logging::error;

void Logging::init(std::string path) {
  Logging::stream = new std::ofstream(path);
  if (path.empty()) {
    Logging::stream->setstate(std::ios_base::badbit);
  }
}

void Logging::cleanup() {
  delete Logging::stream;
  Logging::stream = nullptr;
}

void Logging::breadcrumb(std::string msg) {
  getStream() << "Breadcrumb: " << msg << std::endl;
}

std::ostream &Log::getStream() const {
  return Logging::getStream();
}
