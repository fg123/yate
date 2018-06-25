#ifndef FOCUSABLE_H
#define FOCUSABLE_H

// Represents a entity that can be focused, capturing
// keyboard event and handling it.

class Focusable {
public:
	virtual int capture() = 0;
	virtual void onKeyPress(int key) = 0;
};

#endif
