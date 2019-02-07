#ifndef FILESYSTEM_H
#define FILESYSTEM_H

// Wrapper around OS specific filesystem

#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

class Directory {
  std::string path;
  std::vector<fs::path> path_items;

 public:
  explicit Directory(std::string path) : path(path) {
    path_items.push_back(path + "/..");
    for (auto &p : fs::directory_iterator(path)) {
      path_items.push_back(p.path());
    }
  }
  std::string getDisplayString(size_t index) {
    auto &p = path_items.at(index);
    if (fs::is_directory(p)) {
      return p.filename().string() + "/";
    } else {
      return p.filename().string();
    }
  }
  inline bool isDirectory(size_t index) {
    return fs::is_directory(path_items.at(index));
  }
  const int size() { return path_items.size(); }
  std::string getPath(size_t index) { return path_items.at(index).string(); }
};

#endif  // FILESYSTEM_H
