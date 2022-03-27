#pragma once

#include "../log.hpp"


class Case {
    private:
        bool value;

    public:
        Case(bool value) {
            this->value = value;
        }
        Case() {
            this->value = false;
        }
    
        bool getValue() {
            return this->value;
        }
        void setValue(bool value) {
            this->value = value;
        }

        void toggleValue() {
            this->value = !this->value;
        }
};
