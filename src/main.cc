#include "yate.h"

int main(int argc, char *argv[])
{
	initscr();
	raw();
	start_color();
	keypad(stdscr, true);
	Yate yate;
	endwin();
	return 0;
}
