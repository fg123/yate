#include "filesystem-indexer.h"
#include "logging.h"

#include <chrono>

FilesystemIndexer::FilesystemIndexer() {
  internalThread = std::thread(&FilesystemIndexer::ThreadMain, this);

}

FilesystemIndexer::~FilesystemIndexer() {
  exitThread = true;
  internalThread.join();
}

bool IsDotFolder(const fs::path& path) {
  const std::string& fn = path.filename();
  return fn.size() > 0 && fn[0] == '.';
}

void FilesystemIndexer::ThreadMain() {
  using namespace std::chrono_literals;
  static size_t msCounter = 1000000;

  while (!exitThread) {
    if (msCounter < 5000) continue;
    msCounter = 0;
    std::unordered_set<std::string> newFiles;
    for (fs::recursive_directory_iterator i("."), end; i != end; ++i) {
      if (newFiles.size() > FILES_LIMIT) break;
      if (!fs::is_directory(i->path())) {
        if (IsDotFolder(i->path())) continue;
        newFiles.insert(i->path().filename());
        // Logging::info << i->path().filename() << std::endl;
      }
      else if (IsDotFolder(i->path())) {
        i.disable_recursion_pending();
      }
    }
    fileSetLock.lock();
    std::swap(files, newFiles);
    fileSetLock.unlock();
    std::this_thread::sleep_for(100ms);
    msCounter += 100;
  }
}








