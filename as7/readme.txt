*==========*==========*==========*==========*==========*
**
**      Tyler Carroll
**      CS 381 Assignment 7 README
**      2/8/2026
**      NOTE: I am still having difficulties running ASIO, i've tried re-adding the submodule, changing my cmakelists, but nothing has worked.
**      afaik my code should be functional but I have no clue why my ASIO isn't functional.
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
        No controls, but:
        /nick <name> changes the client's user
        /color <red, green, blue> <message> changes the text of <message> to the correspondign color.

2. Readme question:
    a. The color code can be done client side, since all that needs to happen is the client sending the message with an additional
    escape code in order to denote color. The nick command must be done server side, since that requires the server to know the nicknames of
    all users and to send that nickname before each message made by any user.