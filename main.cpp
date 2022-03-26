#include "game/game.hpp"


int main()
{
    Game game;

    game.setCase(20, 20, true);
    game.setCase(20, 21, true);
    game.setCase(21, 20, true);
    game.setCase(19, 19, true);

    game.run();

    return 0;
}
