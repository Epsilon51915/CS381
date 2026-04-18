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
    