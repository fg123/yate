// An extension on Editor to support display of CSV or tabular files
// Editor and CsvEditor should support switching to and from each
//   other. Thus, CSV files can also be edited in the regular editor.

// All edit, undo / redo happens on the underlying buffer layer, all this
//   class does is a new UI to display the data

#include "editor.h"
#include "csv_buffer.h"

class CsvEditor : public Editor {
  std::string generateStatusBar() override;

  ColNumber current_cell = 0;

  std::vector<std::pair<int, int>> column_widths;
  void populate_widths(BufferWindow window);

public:
  CsvEditor(Yate &yate, Pane *parent, CsvBuffer *buffer,
    int x, int y, int width, int height) : Editor(
      yate, parent, buffer->buffer, x, y, width, height) {
  }

  CsvEditor(Yate &yate, Pane *parent, std::istream &saved_state) :
    Editor(yate, parent, saved_state) {
    Logging::breadcrumb("Deserializing CsvEditor");
  }

  ~CsvEditor() { }

  void draw() override;
  int capture() override;


  void serialize(std::ostream &stream) override {
    stream << "csv_editor " << x << " " << y << " " << width << " " << height << " "
           << buffer->getPath() << " ";
    stream << std::endl;
  }
};
