#include "csv_buffer.h"

void CsvBuffer::repopulateTable() {
  repopulateTable(0, buffer->size());
}

void CsvBuffer::repopulateTable(LineNumber from, LineNumber to) {
  while (internal_table.size() < std::max(from, to)) {
    internal_table.emplace_back(std::vector<std::string>());
  }
  for (LineNumber curr = from; curr < to; curr++) {
    std::string& s = buffer->getLine(curr);

  }
}
