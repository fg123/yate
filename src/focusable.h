#ifndef FOCUSABLE_H
#define FOCUSABLE_H

// Represents a entity that can be focused, capturing
// keyboard event and handling it.

struct Focusable {
	virtual int capture() = 0;
	virtual void onKeyPress(int key) = 0;
};

#endif
