*==========*==========*==========*==========*==========*
**
**      Tyler Carroll
**      CS 381 Assignment 8 README
**      4/12/2026
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
        ./as8
    d. Controls:
        W - Speed up entity in direction it's facing
        S - Slow down entity in direction it's facing
        A - Increase entity yaw/heading
        D- Decrease entity yaw/heading
        R - Increase entity pitch
        F- Decrease entity pitch
        Q - Increase entity roll
        E- Decrease entity roll
        Space - Brings entity to sliding stop
        Tab - Changes selected entity

2. Readme question:
    ECS outperforms CO due to decreasing cache misses by clumping data as tightly as possible, 
    as well as supporting parallelisation by using batch processing on every entity.
    