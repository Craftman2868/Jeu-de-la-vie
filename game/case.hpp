#pragma once

#include "../log.hpp"


class Case {
    private:
        bool value;
        unsigned int x, y;

    public:
        Case(unsigned int x, unsigned int y, bool value) {
            this->x = x;
            this->y = y;
            this->value = value;
        }
        Case(unsigned int x, unsigned int y) {
            this->x = x;
            this->y = y;
            this->value = false;
        }
        Case(bool value) {
            this->x = 0;
            this->y = 0;
            this->value = value;
        }
        Case() {
            this->x = 0;
            this->y = 0;
            this->value = false;
        }
    
        bool getValue() {
            return this->value;
        }
        void setValue(bool value) {
            this->value = value;
        }
    
        int getX() {
            return this->x;
        }
        int getY() {
            return this->y;
        }

        void setPosition(unsigned int x, unsigned int y) {
            this->x = x;
            this->y = y;
        }
};
