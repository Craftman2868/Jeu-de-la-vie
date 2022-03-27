#pragma once

#include <iostream>
#include <unistd.h>
#include <string.h>

#include "case.hpp"
#include "../log.hpp"
#include "render.hpp"

#define GENERATION_TIME 250  // ms
#define FRAME_PAR_GENERATION 2  // no more than 3; no float; no 0; no negative number; (1, 2 or 3)
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
        int selector1_x, selector1_y;
        int selector2_x, selector2_y;
        std::string message;
        // bool updated;
    
    public:
        Game() {
            this->generation = 0;
            this->running = false;
            this->paused = false;
            this->selected_x = WIDTH / 2;
            this->selected_y = HEIGHT / 2;
            this->selector1_x = -1;
            this->selector1_y = -1;
            this->selector2_x = -1;
            this->selector2_y = -1;
            this->message = "";
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
            this->clearMessage();
            // q
            if (ch == 'q') {
                this->running = false;

            // space
            } else if (ch == ' ') {
                this->paused = !this->paused;
                this->setMessage(this->paused ? "Game paused" : "Game resumed");
            
            // ESC
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
                    
                    // alt + u
                    } else if (this->paused && ch == 'u') {
                        this->selector1_x = -1;
                        this->selector1_y = -1;
                        this->selector2_x = -1;
                        this->selector2_y = -1;
                        this->setMessage("Section zone unset");
                    
                    // alt + c
                    } else if (this->paused && ch == 'c') {
                        this->clearSelectedZone();
                        this->setMessage("Section zone cleared");

                    // alt + f
                    } else if (this->paused && ch == 'f') {
                        int n = this->fillSelectedZone();
                        this->setMessage(std::string("Filled ") + std::to_string(n) + " cells");
                    
                    // alt + t
                    } else if (this->paused && ch == 't') {
                        this->toggleSelectedZone();
                        this->setMessage("Section zone toggled");
                    }
                }
            // enter
            } else if (ch == '\n' && this->paused) {
                this->back_cases[this->selected_y][this->selected_x].toggleValue();
                this->setMessage(this->back_cases[this->selected_y][this->selected_x].getValue() ? "Cell alive" : "Cell dead");
            
            // c
            } else if (ch == 'c') {
                this->clear();
                this->setMessage("Game cleared");
            
            // r
            } else if (ch == 'r') {
                this->reset();
                this->setMessage("Game reset");
            
            // n
            } else if (ch == 'n' && this->paused) {
                this->nextGeneration();
                copyBackToFront();
            
            // F
            } else if (ch == 'F' && this->paused) {
                this->fill();
                this->setMessage("Game filled");
            
            // f
            } else if (ch == 'f' && this->paused) {
                int n = this->fillSelectedRegion();
                this->setMessage(std::string("Filled ") + std::to_string(n) + " cells");

            // ctrl + f
            } else if (ch == 0x06 && this->paused) {
                int n = this->fillSelectedRegion(false);
                this->setMessage(std::string("Unfilled ") + std::to_string(n) + " cells");

            // h or ?
            } else if (ch == 'h' || ch == '?') {
                this->help();

            // s
            } else if (this->paused && ch == 's') {
                this->selector1_x = this->selected_x;
                this->selector1_y = this->selected_y;

                this->setMessage("Selector 1 set to " + std::to_string(this->selected_x) + ", " + std::to_string(this->selected_y));
            
            // S
            } else if (this->paused && ch == 'S') {
                this->selector2_x = this->selected_x;
                this->selector2_y = this->selected_y;

                this->setMessage("Selector 2 set to " + std::to_string(this->selected_x) + ", " + std::to_string(this->selected_y));
            
            // u
            } else if (this->paused && ch == 'u') {
                this->selector1_x = -1;
                this->selector1_y = -1;

                this->setMessage("Selector 1 unset");
        
            // U
            } else if (this->paused && ch == 'U') {
                this->selector2_x = -1;
                this->selector2_y = -1;

                this->setMessage("Selector 2 unset");
            
            // other
            } else {
                logDebug("Unrecognized key: " + std::to_string((int) ch));
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

            std::string str_cell = std::to_string(cellNumber);

            str_cell.insert(str_cell.length(), 5 - str_cell.length(), ' ');

            std::cout << " | " << str_cell << " CELLS";

            if (this->message.length() > 0) {
                std::cout << "   \033[2m" << this->message << "\033[0m";
            }

            std::cout << std::endl;

            for (int y = 0; y < HEIGHT; y++) {
                for (int x = 0; x < WIDTH; x++) {
                    if (this->paused && x == this->selected_x && y == this->selected_y) {
                        if (this->getCase(x, y).getValue()) {
                            std::cout << SELECTED_TRUE_CASE;
                        } else {
                            std::cout << SELECTED_FALSE_CASE;
                        }
                    } else if (this->paused && ((x == this->selector1_x && y == this->selector1_y) || (x == this->selector2_x && y == this->selector2_y))) {
                        if (this->getCase(x, y).getValue()) {
                            std::cout << SELECTOR_TRUE_CASE;
                        } else {
                            std::cout << SELECTOR_FALSE_CASE;
                        }
                    } else if (this->paused && this->isInSelectedZone(x, y)) {
                        if (this->getCase(x, y).getValue()) {
                            std::cout << SELECTED_ZONE_TRUE_CASE;
                        } else {
                            std::cout << SELECTED_ZONE_FALSE_CASE;
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

        void setMessage(std::string message) {
            this->message = message;
        }

        void clearMessage() {
            this->message = "";
        }

        void reset() {
            this->generation = 0;
            this->clear();
            this->pause();
        }

        void fill() {
            for (int x = 0; x < WIDTH; x++) {
                for (int y = 0; y < HEIGHT; y++) {
                    this->back_cases[y][x].setValue(true);
                }
            }

            this->copyBackToFront();
        }

        bool isInSelectedZone(int x, int y) {
            if (x >= WIDTH || y >= HEIGHT || x < 0 || y < 0) {
                return false;
            }

            if (this->selector1_x == -1 || this->selector1_y == -1 || this->selector2_x == -1 || this->selector2_y == -1) {
                return false;
            }
            
            int x1, x2, y1, y2;

            if (this->selector1_x < this->selector2_x) {
                x1 = this->selector1_x;
                x2 = this->selector2_x;
            } else {
                x1 = this->selector2_x;
                x2 = this->selector1_x;
            }

            if (this->selector1_y < this->selector2_y) {
                y1 = this->selector1_y;
                y2 = this->selector2_y;
            } else {
                y1 = this->selector2_y;
                y2 = this->selector1_y;
            }

            if (x >= x1 && x <= x2 && y >= y1 && y <= y2) {
                return true;
            }

            return false;
        }

        int fillRegion(int n, int x, int y, bool value) {
            this->setCase(x, y, value);

            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    if (i == 0 && j == 0) {
                        continue;
                    }

                    if (x + i >= 0 && x + i < WIDTH && y + j >= 0 && y + j < HEIGHT) {
                        if (this->getBackCase(x + i, y + j).getValue() != value) {
                            n = this->fillRegion(n + 1, x + i, y + j, value);
                        }
                    }
                }
            }

            return n;
        }

        int fillSelectedRegion(bool value) {
            if (this->getCase(this->selected_x, this->selected_y).getValue() != value) {
                return this->fillRegion(0, this->selected_x, this->selected_y, value);
            }

            return 0;
        }

        int fillSelectedRegion() {
            return this->fillSelectedRegion(true);
        }

        int fillSelectedZone() {
            int n = 0;

            if (this->selector1_x == -1 || this->selector1_y == -1 || this->selector2_x == -1 || this->selector2_y == -1) {
                return 0;
            }

            int x1, x2, y1, y2;

            if (this->selector1_x < this->selector2_x) {
                x1 = this->selector1_x;
                x2 = this->selector2_x;
            } else {
                x1 = this->selector2_x;
                x2 = this->selector1_x;
            }

            if (this->selector1_y < this->selector2_y) {
                y1 = this->selector1_y;
                y2 = this->selector2_y;
            } else {
                y1 = this->selector2_y;
                y2 = this->selector1_y;
            }

            for (int x = x1; x <= x2; x++) {
                for (int y = y1; y <= y2; y++) {
                    if (!this->getCase(x, y).getValue()) {
                        this->setCase(x, y, true);
                        n++;
                    }
                }
            }

            return n;
        }

        void clearSelectedZone() {
            if (this->selector1_x == -1 || this->selector1_y == -1 || this->selector2_x == -1 || this->selector2_y == -1) {
                return;
            }

            int x1, x2, y1, y2;

            if (this->selector1_x < this->selector2_x) {
                x1 = this->selector1_x;
                x2 = this->selector2_x;
            } else {
                x1 = this->selector2_x;
                x2 = this->selector1_x;
            }

            if (this->selector1_y < this->selector2_y) {
                y1 = this->selector1_y;
                y2 = this->selector2_y;
            } else {
                y1 = this->selector2_y;
                y2 = this->selector1_y;
            }

            for (int x = x1; x <= x2; x++) {
                for (int y = y1; y <= y2; y++) {
                    if (this->getCase(x, y).getValue()) {
                        this->setCase(x, y, false);
                    }
                }
            }
        }

        void toggleSelectedZone() {
            if (this->selector1_x == -1 || this->selector1_y == -1 || this->selector2_x == -1 || this->selector2_y == -1) {
                return;
            }

            int x1, x2, y1, y2;

            if (this->selector1_x < this->selector2_x) {
                x1 = this->selector1_x;
                x2 = this->selector2_x;
            } else {
                x1 = this->selector2_x;
                x2 = this->selector1_x;
            }

            if (this->selector1_y < this->selector2_y) {
                y1 = this->selector1_y;
                y2 = this->selector2_y;
            } else {
                y1 = this->selector2_y;
                y2 = this->selector1_y;
            }

            for (int x = x1; x <= x2; x++) {
                for (int y = y1; y <= y2; y++) {
                    if (this->getCase(x, y).getValue()) {
                        this->setCase(x, y, false);
                    } else {
                        this->setCase(x, y, true);
                    }
                }
            }
        }

        void help() {
            this->pause();

            std::cout << "\033[2J\033[1;1H";

            std::cout << std::endl;
            std::cout << "     ====================================" << std::endl;
            std::cout << "     =============[   \033[1mHelp\033[0m   ]===========" << std::endl;
            std::cout << "     ====================================" << std::endl;
            std::cout << "     \033[2m<ctrl>\033[0m + \033[2m<c>\033[0m  force quit"             << std::endl;
            std::cout << "              \033[2m<q>\033[0m  quit"                   << std::endl;
            std::cout << "          \033[2m<space>\033[0m  pause"                  << std::endl;
            std::cout << "             \033[2m<up>\033[0m  move selector up"       << std::endl;
            std::cout << "           \033[2m<down>\033[0m  move selector down"     << std::endl;
            std::cout << "           \033[2m<left>\033[0m  move selector left"     << std::endl;
            std::cout << "          \033[2m<right>\033[0m  move selector right"    << std::endl;   
            std::cout << "          \033[2m<enter>\033[0m  change case value"      << std::endl;
            std::cout << "              \033[2m<c>\033[0m  clear"                  << std::endl;
            std::cout << "              \033[2m<r>\033[0m  reset"                  << std::endl;
            std::cout << "              \033[2m<n>\033[0m  next"                   << std::endl;
            std::cout << "              \033[2m<F>\033[0m  fill game"              << std::endl;
            std::cout << "              \033[2m<f>\033[0m  fill selected region"   << std::endl;
            std::cout << "     \033[2m<ctrl>\033[0m + \033[2m<f>\033[0m  unfill selected region" << std::endl;
            std::cout << "       \033[2m<h>\033[0m or \033[2m<?>\033[0m  help"                    << std::endl;
            std::cout << "     ====================================" << std::endl;
            std::cout << std::endl;
            std::cout << "      Press any key to return to game..."  << std::endl;

            while (!getCharNonBlock());
        }
};
