*==========*==========*==========*==========*==========*
**
**      Tyler Carroll
**      CS 381 Assignment 2 README
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
        ./as1
    Mouse controls used for sliders

2. Readme question:
    Audio is produced by a speaker through pressure waves made by moving cones within the speaker, moved with a motor whose intensity is determined
    by the volume and sound being played.
    The raylib Audio Device is necessary because it determines where to send sounds to, routing to your system's default audio output. Without
    this, no sound would be heard.