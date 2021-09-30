#ifndef FILESYSTEM_INDEXER_H
#define FILESYSTEM_INDEXER_H

#include <filesystem>
#include <thread>
#include <mutex>
#include <unordered_set>
#include <string>

namespace fs = std::filesystem;

const static size_t FILES_LIMIT = 1000;

// Provides filenames in the Navigate Menu, even for files
//   that are not opened, this indexes recursively all the files
//   in a separate thread
class FilesystemIndexer {

  bool exitThread = false;
  size_t versionInfo;
  std::thread internalThread;
public:
  std::mutex fileSetLock;
  std::unordered_set<std::string> files;

  FilesystemIndexer();
  ~FilesystemIndexer();

  void ThreadMain();

};


#endif
