#pragma once

#include <sys/select.h>
#include <termios.h>
#include <string.h>
#include <curses.h>

#define TRUE_CASE "()"
#define FALSE_CASE "``"
#define SELECTED_TRUE_CASE "\033[1;41m" TRUE_CASE "\033[0m"
#define SELECTED_FALSE_CASE "\033[1;41m" FALSE_CASE "\033[0m"

struct termios orig_termios;

void reset_terminal_mode()
{
    tcsetattr(0, TCSANOW, &orig_termios);
}

void set_conio_terminal_mode()
{
    struct termios new_termios;

    /* take two copies - one for now, one for later */
    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));

    /* register cleanup handler, and set the new terminal mode */
    atexit(reset_terminal_mode);
    cfmakeraw(&new_termios);
    tcsetattr(0, TCSANOW, &new_termios);
}

int kbhit()
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv) > 0;
}

char getCharNonBlock()
{
    set_conio_terminal_mode();

    if (kbhit()) {
        char c;
        read(STDIN_FILENO, &c, sizeof(char));
        reset_terminal_mode();
        return c;
    }

    reset_terminal_mode();

    return 0;
}
