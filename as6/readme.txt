*==========*==========*==========*==========*==========*
**
**      Tyler Carroll
**      CS 381 Assignment 6 README
**      2/8/2026
**
*==========*==========*==========*==========*==========*

1. Commands to compile/run program

    a. Fetch repository
        git submodule add 
        git submodule --init --recursive

    b. Compile program (starting within the as1 folder):
    IMPORTANT: DELETE ANY PREVIOUSLY EXISTING BUILD FOLDER
        mkdir build
        cd build
        cmake ..
        make

    c. Run program
        ./as6
    d. Controls:
        A/D - Change the direction that the penguin is spinning in.
        Mouse controls for volume slider for music.