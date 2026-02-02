#include <iostream>
#include "raylib-cpp.hpp"
#include "raylib.h"

int main()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE); 
    raylib::Window window(600, 400, "CS 381 - Assignment 0");

    int fontSize = 35;
    
    while(!window.ShouldClose())
    {
        int textW = MeasureText("Hello, World!", fontSize);
        int textX = (window.GetWidth() / 2) - (textW / 2);
        int textY = (window.GetHeight() / 2) - (fontSize / 2);


        DrawText("Hello, World!", textX, textY, 35, RAYWHITE);
        ClearBackground(BLACK);
        window.BeginDrawing();
        window.EndDrawing();
    }
    window.Close();
    return 0;
}