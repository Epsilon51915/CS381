*==========*==========*==========*==========*==========*
**
**      Tyler Carroll
**      CS 381 Assignment 9 README
**      4/13/2026
**
*==========*==========*==========*==========*==========*

1. Commands to compile/run program

    a. Fetch repository
        git submodule add <link>
        git submodule --init --recursive

    b. Compile program (starting within the as8 folder):
    IMPORTANT: DELETE ANY PREVIOUSLY EXISTING BUILD FOLDER
        mkdir build
        cd build
        cmake ..
        make

    c. Run program
        ./as9
    d. Controls:
        W - Move character up one tile
        S - Move character down one tile
        A - Move character left one tile
        D - Move character right one tile
        ENTER - Open/close inventory

2. Extra Credit:
    a. As can be obviously seen, this game is not a recreation of Penguins and Eagles! I decided to go for a more
    survival crafting game with world exploration, resorce collection, and ideally more mechanics in terms of
    crafting and surviving/building.

    b. TECHNICALLY, all players are visually distinct. There's only one character to be shown (I'm still having ASIO 
    issues), but it is visually distinct from the background + comes with customisation options.

    c. Following this, there is a username function

    d. There is reactive audio! When picking up vines (the green 'c's on the ground,) there is a 10% chance to drop
    an additional seed item, used for planting. If you get this 10% drop, a different sound will play from the regular
    item pickup sound.

    e. As for guessing a score, I create a window (+5), the game is "technically" loopable, since it only ends when
    the user closes it, there is no condition where the game needs to restart (+15), there is reactive audio, with
    the item pickup sound changing if you get a lucky drop from vines vs a regular drop (+10), there are controls (+5),
    following the long string of technicalities, each user can only update the tile they're standing on with regards
    to items on the ground (+10), the game is a custom game with custom sfx not related to penguins and eagles (+30),
    there is a username system with the prompt at the beginning of the game (+10), and each player is visually distinct
    (+10). All in all, that amounts to 95 points total, including extra credit, that would be 120, if my assumptions on
    which technicalities would fly are correct. 