*==========*==========*==========*==========*==========*
**
**      Tyler Carroll
**      CS 381 Assignment 3 README
**      2/8/2026
**
**      PLEASE NOTE: Issue where when drawing skybox, the penguin takes on the skybox texture. I'm not quite sure how to fix this.
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
        ./as3
    d. Controls:
        W - Speed up penguin in direction it's facing
        S - Slow down penguin in direction it's facing
        A - Rotate penguin right
        D- Rotate pengin left
        Left Shift - Sets penguins target velocity to zero; Penguin will slow to a stop.
        Tab - Sets penguin's position back to the origin (0, 0, 0)
        Space - Sets penguin's velocity to 0; Stops the penguin COMPLETELY
        Enter - Toggles between hold and buffer mode: Hold mode allows user to hold an input instead of requiring user to tap input         buttons. This setting is off by default.

2. Readme question:
    a. DT stands for Delta Time, and is important in ensuring consistent motion across different framerates. It is calculated but finding the elapsed time between the previous frame and the one currently being processed.

3. Extra Credit:
    a. Extra controls! I took the liberty of adding a sliding stop and toggles for holding inputs (mainly because I didn't want to keep tapping buttons in order to make the penguin move,) as well as a position resetter, in case the penguin ends up off screen.
    b. Wrapping! If the penguin moves too far to one side of the screen, it will wrap back around to the other side of the screen! 