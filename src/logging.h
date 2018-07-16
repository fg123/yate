#ifndef LOGGING_H
#define LOGGING_H

#include <fstream>
#include <string>

class Log {
  std::ostream &getStream() const;

 public:
  template <class T>
  Log &operator<<(const T &msg) {
    getStream() << msg;
    return *this;
  }

  Log const &operator<<(std::ostream &(*F)(std::ostream &)) const {
    F(getStream());
    return *this;
  }
};

class Logging {
  static std::ostream *stream;

 public:
  static std::ostream &getStream() { return *stream; }
  static Log info;
  static Log error;
  static void init(std::string logging_path);
  static void breadcrumb(std::string msg);
  static void cleanup();
};

#endif
