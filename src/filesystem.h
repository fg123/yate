#ifndef FILESYSTEM_H
#define FILESYSTEM_H

// Wrapper around OS specific filesystem

#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

class Directory {
  std::string path;
  std::vector<std::string> items;

 public:
  explicit Directory(std::string path) : path(path) {
    for (auto &p : fs::directory_iterator(path)) {
      if (fs::is_directory(p.path())) {
        items.push_back(p.path().filename().string() + "/");
      } else {
        items.push_back(p.path().filename().string());
      }
    }
  }
  std::vector<std::string> &getListing() { return items; }
};

#endif  // FILESYSTEM_H
