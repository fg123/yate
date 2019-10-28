#ifndef CSV_BUFFER_H
#define CSV_BUFFER_H

#include "buffer.h"

class CsvBuffer {
  char delim = ',';

public:
  Buffer* buffer;

  CsvBuffer(Buffer* buffer) : buffer(buffer) {
    repopulateTable();
  }

  void repopulateTable();
  void repopulateTable(LineNumber from, LineNumber to);
};

#endif
