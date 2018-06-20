#include "yate.h"

int main(int argc, char *argv[])
{
	initscr();
	raw();
	noecho();
	start_color();
	keypad(stdscr, true);
	Yate yate;
	endwin();
	return 0;
}
