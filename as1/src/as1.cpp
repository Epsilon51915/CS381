#include "raylib-cpp.hpp"
#include "raylib.h"
#include <iostream>
#include <optional>
#include<functional>

std::optional<std::function<void()>> glob;

// Callback function
void PingButton()
{
    
    //pings.Play(); 
    std::cout << " yeowch" << std::endl;

    (*glob)();
}

#define GUI_VOLUMECONTROL_IMPLEMENTATION
#include "VolumeControl.h"

int main()
{
    //SetConfigFlags(FLAG_WINDOW_RESIZABLE); 
    raylib::Window window(300, 350, "CS 381 - Assignment 1");
    raylib::AudioDevice audio;
    auto gui_state = InitGuiVolumeControl();

    
    raylib::Music music("audio/ping.mp3");
    raylib::Music dialogue("audio/fred.mp3");
    music.Play();
    dialogue.Play();
    raylib::Sound pings("audio/fah.mp3");
    
    glob = [&pings](){
        
        pings.Play();
    };

    while(!window.ShouldClose())
    {
        window.BeginDrawing();
        ClearBackground(BLACK);

        GuiVolumeControl(&gui_state);
        window.EndDrawing();

        dialogue.SetVolume(gui_state.DialogueSliderValue/100);

        music.SetVolume(gui_state.MusicSliderValue/100);
        pings.SetVolume(gui_state.SFXSliderValue/100);
        if(gui_state.MusicSliderValue != 0)
        {
            music.Update();
        }
        dialogue.Update();
        
    }

}