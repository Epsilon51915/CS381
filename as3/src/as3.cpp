#include "raylib-cpp.hpp"
#include "raylib.h"
#include "AudioDevice.hpp"
#include "Camera3D.hpp"
#include "Matrix.hpp"
#include "Model.hpp"
#include "RadiansDegrees.hpp"
#include <iostream>

#define SKYBOX_IMPLEMENTATION

#include "skybox.hpp"

void DrawBoundedModel(raylib::Model& model, auto transformer)
{    
    // Stores original transform info
    raylib::Transform backupTransform = model.transform;

    // Applies new transform
    model.transform = transformer(backupTransform);

    // Draws model with new transforms at default coordinates
    model.Draw({0, 0, 0});

    // Retrieves the bounding box from the transformed model, not the original model
    raylib::BoundingBox box = model.GetTransformedBoundingBox();

    // Draws the transformed bounding box with respect to the transformed model with the chosen color
    DrawBoundingBox(box, raylib::Color::Blue());

    // Restores old transform info back to the model after drawing
    model.transform = backupTransform;
}

raylib::Degree angle_clamp(raylib::Degree angle)
{
    float decimal = float(angle) - int(angle);
    int whole = int(angle) % 360;
    whole += (angle < 0) * 360;
    return decimal + whole;
}

int main()
{
    raylib::Window window(800, 600, "AS3");
    window.SetState(FLAG_WINDOW_RESIZABLE);
    raylib::AudioDevice audio;
    
    raylib::Model penguin("models/penguin.glb");
    penguin.transform = raylib::Transform(penguin.transform).Scale(30);
    raylib::Camera camera({0, 120, 500}, {0, 0, 0});
    
    raylib::Texture snow("textures/snow.jpg");
    raylib::Model ground = raylib::Mesh::Plane(10000, 10000, 50, 50, 25).LoadModelFrom();
    ground.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = snow;

    raylib::Vector3 position = {0, 0, 0};
    raylib::Vector3 velocity = {0, 0, 0};

    raylib::Degree heading = 0;
    float speed = 0;
    float target_speed = 0;
    const float acc = 10;

    cs381::SkyBox skybox("textures/skybox.png");

    bool hold_mode = false;
    bool wait = false;

    while(!window.ShouldClose())
    {
        window.BeginDrawing();
        {
            float dt = window.GetFrameTime();
            window.ClearBackground(raylib::Color::RayWhite());

            if(raylib::Keyboard::IsKeyPressed(KEY_ENTER) && !hold_mode)
            {
                hold_mode = true;
                //std::cout << "Hold mode on" << std::endl;
                wait = true;
            }
            if(raylib::Keyboard::IsKeyPressed(KEY_ENTER) && hold_mode && !wait)
            {
                hold_mode = false;
                //std::cout << "Hold mode off" << std::endl;
            }
            wait = false;

            if(!hold_mode)
            {
                if(raylib::Keyboard::IsKeyPressed(KEY_W))
                {
                    target_speed += 10;
                }
                if(raylib::Keyboard::IsKeyPressed(KEY_S))
                {
                    target_speed -= 10;
                }
                if(raylib::Keyboard::IsKeyPressed(KEY_A))
                {
                    heading -= 10;
                }
                if(raylib::Keyboard::IsKeyPressed(KEY_D))
                {
                    heading += 10;
                }
            }
            else
            {
                if(raylib::Keyboard::IsKeyDown(KEY_W))
                {
                    target_speed += 1;
                }
                if(raylib::Keyboard::IsKeyDown(KEY_S))
                {
                    target_speed -= 1;
                }
                if(raylib::Keyboard::IsKeyDown(KEY_A))
                {
                    heading -= 1;
                }
                if(raylib::Keyboard::IsKeyDown(KEY_D))
                {
                    heading += 1;
                }
            }

            
            if(raylib::Keyboard::IsKeyPressed(KEY_LEFT_SHIFT))
            {
                target_speed = 0;
            }

            heading = angle_clamp(heading);
            

            if(speed < target_speed)
            {
                speed += acc * dt;
            }
            else if (speed > target_speed)
            {
                speed -= acc * dt;
            }

            velocity = raylib::Vector3(-cos(heading.RadianValue()), 0, sin(heading.RadianValue())) * speed;

            if(raylib::Keyboard::IsKeyPressed(KEY_TAB))
            {
                position.SetX(0);
                position.SetY(0);
                position.SetZ(0);
            }
            if(raylib::Keyboard::IsKeyPressed(KEY_SPACE))
            {   
                speed = 0;
                target_speed = 0;
                velocity = 0;
            }

            position += velocity * dt;

            // Wrapping
            if(position.x > 350)
            {
                position.x = -350;
            }
            if(position.x < -350)
            {
                position.x = 350;
            }
            if(position.z > 500)
            {
                position.z = -500;
            }
            if(position.z < -500)
            {
                position.z = 500;
            }

            camera.BeginMode();
            {
                DrawBoundedModel(penguin, [&position, &heading](raylib::Transform& transform) -> raylib::Transform
                {
                    return transform.Translate(position).RotateY(raylib::Degree(heading));
                });
                skybox.Draw();
                ground.Draw({});
                //penguin.Draw({});
            }
            camera.EndMode();
        }
        window.EndDrawing();
    }
}