#pragma once

#include <iostream>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>
#include <string.h>
#include <curses.h>

#include "case.hpp"
#include "../log.hpp"


#define GENERATION_TIME 250  // ms
#define FRAME_PAR_GENERATION 2  // no more than 2; no float
#define WIDTH 50
#define HEIGHT 100
#define TRUE_CASE "()"
#define FALSE_CASE "  "
#define SELECTED_TRUE_CASE "[]"
#define SELECTED_FALSE_CASE "<>"

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

class Game {
    private:
        Case front_cases[HEIGHT][WIDTH];
        Case back_cases[HEIGHT][WIDTH];
        int generation;
        bool running;
        bool paused;
        int selected_x, selected_y;
        // bool updated;
    
    public:
        Game() {
            this->generation = 0;
            this->running = false;
            this->paused = false;
            this->selected_x = WIDTH / 2;
            this->selected_y = HEIGHT / 2;
            // this->updated = false;

            for (int y = 0; y < HEIGHT; y++) {
                for (int x = 0; x < WIDTH; x++) {
                    this->back_cases[y][x].setPosition(x, y);
                }
            }

            for (int y = 0; y < HEIGHT; y++) {
                for (int x = 0; x < WIDTH; x++) {
                    this->front_cases[y][x].setPosition(x, y);
                }
            }
        }

        ~Game() {
            for (int i = 0; i < HEIGHT; i++) {
                for (int j = 0; j < WIDTH; j++) {
                    this->front_cases[i][j] = NULL;
                }
            }

            for (int i = 0; i < HEIGHT; i++) {
                for (int j = 0; j < WIDTH; j++) {
                    this->back_cases[i][j] = NULL;
                }
            }
        }

        void run() {
            char ch;

            this->running = true;
            this->paused = false;

            while (this->running) {
                copyBackToFront();

                int frame_in_current_generation = 0;
                while (frame_in_current_generation < FRAME_PAR_GENERATION) {
                    ch = getCharNonBlock();

                    if (ch) {
                        // this->updated = true;
                        this->interpretInput(ch);
                    }

                    this->print();

                    frame_in_current_generation++;
                    usleep(GENERATION_TIME * 1000 / FRAME_PAR_GENERATION);
                }

                if (!this->paused) {
                    this->nextGeneration();
                }
            }
        }

        void interpretInput(char ch) {
            if (ch == 'q') {
                this->running = false;
            } else if (ch == ' ')
            {
                this->paused = !this->paused;
            } else if (ch == 27) {
                if (kbhit) {
                    ch = getCharNonBlock();
                    if (ch == '[') {
                        ch = getCharNonBlock();
                        if (ch == 'A' and this->selected_x < HEIGHT - 1) {
                            this->selected_x--;
                        } else if (ch == 'B' and this->selected_x > 0) {
                            this->selected_x++;
                        } else if (ch == 'C' and this->selected_y < WIDTH - 1) {
                            this->selected_y++;
                        } else if (ch == 'D' and this->selected_y > 0) {
                            this->selected_y--;
                        }
                    }
                }
            }
        }

        void print() {
            // if (!this->updated) {
            //     return;
            // }
            std::cout << "\033[2J\033[1;1H";
            std::cout << "GENERATION " << this->generation;
            if (this->paused) {
                std::cout << " (PAUSED)" << std::endl;
            } else {
                std::cout << std::endl;
            }
            for (int x = 0; x < WIDTH; x++) {
                for (int y = 0; y < HEIGHT; y++) {
                    if (x == this->selected_x && y == this->selected_y) {
                        if (this->getCase(x, y).getValue()) {
                            std::cout << SELECTED_TRUE_CASE;
                        } else {
                            std::cout << SELECTED_FALSE_CASE;
                        }
                    } else {
                        if (this->getCase(x, y).getValue()) {
                            std::cout << TRUE_CASE;
                        } else {
                            std::cout << FALSE_CASE;
                        }
                    }
                }
                std::cout << std::endl;
            }
            // this->updated = false;
        }

        void nextGeneration() {
            this->generation++;

            for (int x = 0; x < WIDTH; x++) {
                for (int y = 0; y < HEIGHT; y++) {
                    int8_t nb = this->getNbNeighbors(x, y);

                    if (this->getCase(x, y).getValue()) {
                        if (nb < 2 || nb > 3) {
                            this->setCase(x, y, false);
                        }
                    } else {
                        if (nb == 3) {
                            this->setCase(x, y, true);
                        }
                    }
                }
            }
        }

        void setCase(int x, int y, Case c) {
            if (x >= WIDTH || y >= HEIGHT || x < 0 || y < 0) {
                return;
            }

            this->back_cases[y][x] = c;
            // this->updated = true;
        }
        void setCase(int x, int y, bool value) {
            if (x >= WIDTH || y >= HEIGHT || x < 0 || y < 0) {
                return;
            }

            this->back_cases[y][x].setValue(value);
            // this->updated = true;
        }
        Case getCase(int x, int y) {
            if (x >= WIDTH || y >= HEIGHT || x < 0 || y < 0) {
                return Case();
            }

            return this->front_cases[y][x];
        }

        int getGeneration() {
            return this->generation;
        }

        int8_t getNbNeighbors(unsigned int x, unsigned int y) {
            Case c = this->getCase(x, y);

            int8_t nb = 0;

            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    if (i == 0 && j == 0) {
                        continue;
                    }

                    if (this->getCase(x + i, y + j).getValue()) {
                        nb++;
                    }
                }
            }

            // if (c.getValue()) {
            //     logDebug("Case (" + std::to_string(x) + ", " + std::to_string(y) + ") has got " + std::to_string(nb) + " neighbors");
            // }

            return nb;
        }

        void copyBackToFront() {
            for (int y = 0; y < HEIGHT; y++) {
                for (int x = 0; x < WIDTH; x++) {
                    this->front_cases[y][x].setValue(this->back_cases[y][x].getValue());
                }
            }
        }
};
