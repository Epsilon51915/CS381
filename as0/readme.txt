*==========*==========*==========*==========*==========*
**
**      Tyler Carroll
**      CS 381 Assignment 0 README
**      2/1/2026
**
*==========*==========*==========*==========*==========*

    1. Code function
        a. To make a window resizeable with raylib, we use the SetConfigFlags method before creating a window. To do this, we simply put
            SetConfigFlags(FLAG_WINDOW_RESIZEABLE) before creating our window.
        b. To keep text centered in a window, we must first know the content of the text, its size, and our window size. With this knowledge,
            we can find the desired X position of the text using raylib's GetWidth() method for windows, halve this value, and then subtract half 
            of our text width. We use a similar process to find the Y position for the text, instead using raylib's GetHeight() method. Finally,
            when drawing the text, we use raylib's DrawText() method, being sure to specify the text's content, X and Y positions, fint size, and
            font color.
        c. The command 'cmake ..' runs the CMakeLists.txt file in the folder above the current working directory. The '..' in the command tells
            the compiler that they should look one folder above the cwd, instead of in the cwd itself.
        d. In order to fetch git submodules, we use the git submodule add <link> command. This initial command creates the submodule, and clones
            the provided repository into that submodule. After, we use git submodule update --init --recursive in order to populate all of the 
            folders within the submodule and ensure they are ready for usage.
        e. In order to compile the code, first delete any build folder present. Create a new build folder named 'build' within the same directory
            using 'mkdir build'. Change to this directory ('cd build') and run 'cmake ..'. Finally, run 'make' to run the program.
        f. In order to run the program, within the build directory, simply run ./as0 to run the program


2. All commands grouped:
    mkdir build
    cd build
    cmake ..
    make
    ./as0
