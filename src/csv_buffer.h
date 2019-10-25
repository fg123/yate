#ifndef CSV_BUFFER_H
#define CSV_BUFFER_H

#include "buffer.h"

class CsvBuffer {
  Buffer* buffer;
public:
  CsvBuffer(Buffer* buffer) : buffer(buffer) {
    // We don't own buffer so we
  }
};

#endif
