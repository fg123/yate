#include "yate.h"

int main(int argc, char *argv[])
{
	initscr();
	raw();
	noecho();
	nonl();
	start_color();
	keypad(stdscr, true);
	Yate yate;
	endwin();
	return 0;
}
