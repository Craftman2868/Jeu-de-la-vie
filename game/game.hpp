#pragma once

#include <iostream>
#include <unistd.h>

#include "case.hpp"
#include "../log.hpp"
#include "render.hpp"

#define GENERATION_TIME 250  // ms
#define FRAME_PAR_GENERATION 2  // no more than 2; no float
#define WIDTH 50
#define HEIGHT 50


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
            this->selected_y = HEIGHT / 2;
            this->selected_x = WIDTH / 2;
            // this->updated = false;
        }

        ~Game() {
            for (int x = 0; x < WIDTH; x++) {
                for (int y = 0; y < HEIGHT; y++) {
                    this->front_cases[y][x] = NULL;
                }
            }

            for (int x = 0; x < WIDTH; x++) {
                for (int y = 0; y < HEIGHT; y++) {
                    this->back_cases[y][x] = NULL;
                }
            }
        }

        void run() {
            char ch;
            bool isEmpty;

            this->running = true;

            while (this->running) {
                if (!this->paused) {
                    isEmpty = this->nextGeneration();
                    if (!copyBackToFront()) {
                        this->pause();
                    }
                }

                if (isEmpty && !this->paused) {
                    this->pause();
                }

                int frame_in_current_generation = 0;
                while (frame_in_current_generation < FRAME_PAR_GENERATION) {
                    ch = getCharNonBlock();

                    if (ch) {
                        // this->updated = true;
                        this->interpretInput(ch);
                        copyBackToFront();
                    }

                    this->print();

                    frame_in_current_generation++;
                    usleep(GENERATION_TIME * 1000 / FRAME_PAR_GENERATION);
                }
            }
        }

        void interpretInput(char ch) {
            if (ch == 'q') {
                this->running = false;
            } else if (ch == ' ') {
                this->paused = !this->paused;
            } else if (ch == 27) {
                if (kbhit) {
                    ch = getCharNonBlock();
                    if (ch == '[') {
                        ch = getCharNonBlock();
                        if (ch == 'A' && this->selected_y > 0) {
                            this->selected_y--;
                        } else if (ch == 'B' && this->selected_y < HEIGHT - 1) {
                            this->selected_y++;
                        } else if (ch == 'C' && this->selected_x < WIDTH - 1) {
                            this->selected_x++;
                        } else if (ch == 'D' && this->selected_x > 0) {
                            this->selected_x--;
                        } else {
                            return;
                        }

                        this->paused = true;
                    }
                }
            } else if (ch == '\n' && this->paused) {
                this->back_cases[this->selected_y][this->selected_x].toggleValue();
            } else if (ch == 'c') {
                this->clear();
            } else if (ch == 'r') {
                this->reset();
            } else if (ch == 'n' && this->paused) {
                this->nextGeneration();
                copyBackToFront();
            }
        }

        void print() {
            // if (!this->updated) {
            //     return;
            // }
            int cellNumber = this->countCells();

            std::cout << "\033[2J\033[1;1H";

            std::string str_gen = std::to_string(this->generation);

            str_gen.insert(str_gen.length(), 6 - str_gen.length(), ' ');

            std::cout << "GENERATION " << str_gen;
            if (this->paused) {
                std::cout << "    (PAUSED)";
            } else {
                std::cout << "            ";
            }
            std::cout << " | " << cellNumber << " CELLS" << std::endl;
            for (int y = 0; y < HEIGHT; y++) {
                for (int x = 0; x < WIDTH; x++) {
                    if (this->paused && x == this->selected_x && y == this->selected_y) {
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

        int countCells() {
            int count = 0;
            for (int x = 0; x < WIDTH; x++) {
                for (int y = 0; y < HEIGHT; y++) {
                    if (this->getCase(x, y).getValue()) {
                        count++;
                    }
                }
            }
            return count;
        }

        bool nextGeneration() {
            bool isEmpty = true;

            this->generation++;

            for (int x = 0; x < WIDTH; x++) {
                for (int y = 0; y < HEIGHT; y++) {
                    int8_t nb = this->getNbNeighbors(x, y);

                    if (this->getCase(x, y).getValue()) {
                        if (nb < 2 || nb > 3) {
                            this->setCase(x, y, false);
                        }
                        isEmpty = false;
                    } else {
                        if (nb == 3) {
                            this->setCase(x, y, true);
                        }
                    }
                }
            }

            return isEmpty;
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
        Case getBackCase(int x, int y) {
            if (x >= WIDTH || y >= HEIGHT || x < 0 || y < 0) {
                return Case();
            }

            return this->back_cases[y][x];
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

            return nb;
        }

        bool copyBackToFront() {
            bool isDifferent = false;

            for (int x = 0; x < WIDTH; x++) {
                for (int y = 0; y < HEIGHT; y++) {
                    if (front_cases[y][x].getValue() != this->back_cases[y][x].getValue()) {
                        this->front_cases[y][x].setValue(this->back_cases[y][x].getValue());
                        isDifferent = true;
                    }
                }
            }

            return isDifferent;
        }

        void clear() {
            for (int x = 0; x < WIDTH; x++) {
                for (int y = 0; y < HEIGHT; y++) {
                    this->back_cases[y][x].setValue(false);
                }
            }

            this->copyBackToFront();
        }

        void pause() {
            this->paused = true;
        }

        void resume() {
            this->paused = false;
        }

        void togglePause() {
            this->paused = !this->paused;
        }

        void reset() {
            this->generation = 0;
            this->clear();
            this->pause();
        }
};
