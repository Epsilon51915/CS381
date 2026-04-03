*==========*==========*==========*==========*==========*
**
**      Tyler Carroll
**      CS 381 Assignment 7 README
**      2/8/2026
**      NOTE: I must've linked the ASIO library incorrectly, and I didn't notice until I got back from break, so this initial post only contains
**          the comments for the client and server while I try to re-link ASIO to the assignment.
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
        ./as5
    d. Controls:
        W - Speed up penguin/eagle in direction it's facing
        S - Slow down penguin/eagle in direction it's facing
        A - Rotate penguin right/Rotate eagle CCW
        D- Rotate pengin left/Rotate eagle CW
        Left Shift - Sets entity's target speed, speed, and velocity to 0, stopping it INSTANTLY
        Tab - Changes selected entity
        Space - Sets entity's target velocity to 0; Entity will slow to a stop
        Right Shift - Sets entity's position to it's origin (0, 0, 0) for penguins, (0, 150, 0) for eagles

2. Readme question:
    a. Separating functions into Draw and Draw Bounded Model is the most scalable, but requires the most complexity with implementation, using the bool
    within the struct is the best for low amounts of objects, since each bool stored increases required bytes by 1 per entity, and calculating based
    off of the selected value is a good middle ground with storing less data but also not being too complex to implement.

3. Extra Credit:
    a. Extra controls! I took the liberty of adding a hard stop and position resetting for all entities
    b. Wrapping! If an entity moves too far to one side of the screen, it will wrap back around to the other side of the screen! 