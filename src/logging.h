#ifndef LOGGING_H
#define LOGGING_H

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <queue>
#include <iomanip>
#include <ctime>

#define LOG_TIME_FORMAT "%OH:%OM:%OS "

struct LogListener {
  virtual void onMessage(std::string message) = 0;
  virtual void onFlushed() = 0;
};

class _Log;
struct Logging {
  static std::vector<std::ostream*> streams;
  static std::vector<LogListener*> listeners;

  static std::queue<std::string> buffer;

  static _Log info;
  static _Log error;
  static void init(std::string logging_path);
  static void breadcrumb(std::string msg);
  static void cleanup();
  static void addListener(LogListener *stream) {
    listeners.push_back(stream);
  }
  static void removeListener(LogListener *listener) {
    listeners.erase(std::remove(listeners.begin(),
      listeners.end(), listener), listeners.end());
  }
  static void flush() {
    size_t size = buffer.size();
    // We have to use this because listeners might push to the
    //   buffer, and this loop would never end.
    for (size_t i = 0; i < size; i++) {
      std::string s = buffer.front();
      for (auto& listener : Logging::listeners) {
        listener->onMessage(s);
      }
      buffer.pop();
    }
    for (auto& listener : Logging::listeners) {
      listener->onFlushed();
    }
  }
};

static std::time_t time_now = std::time(nullptr);

class Log {
public:
  template <class T>
  Log &operator<<(const T &msg) {
    for (auto stream : Logging::streams) {
      *stream << msg;
    }
    std::ostringstream o;
    o << msg;
    Logging::buffer.push(std::move(o.str()));
    return *this;
  }

  Log const &operator<<(std::ostream &(*F)(std::ostream &)) const {
    for (auto stream : Logging::streams) {
      F (*stream);
    }
    std::ostringstream o;
    F(o);
    Logging::buffer.push(std::move(o.str()));
    return *this;
  }
};

class _Log : Log {
 public:
  template <class T>
  Log &operator<<(const T &msg) {
    auto time =
        std::put_time(std::localtime(&time_now), LOG_TIME_FORMAT);
    Log::operator<<(time);
    Log::operator<<(msg);
    return *this;
  }
};
#endif
