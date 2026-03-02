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

struct Entity
{
    raylib::Vector3 position = {0, 0, 0};
    raylib::Degree heading = 0;
    float speed = 0;
    float target_speed = 0;
    raylib::Model* model;
    bool isEagle = false;
};

void DrawBoundedModel(raylib::Model& model, bool drawBB, auto transformer)
{    
    // Stores original transform info
    raylib::Transform backupTransform = model.transform;

    // Applies new transform
    model.transform = transformer(backupTransform);

    // Draws model with new transforms at default coordinates
    model.Draw({0, 0, 0});

    if(drawBB)
    {
        // Retrieves the bounding box from the transformed model, not the original model
        raylib::BoundingBox box = model.GetTransformedBoundingBox();

        // Draws the transformed bounding box with respect to the transformed model with the chosen color
        DrawBoundingBox(box, raylib::Color::Blue());
    }
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
    raylib::Window window(800, 600, "AS4");
    window.SetState(FLAG_WINDOW_RESIZABLE);
    raylib::AudioDevice audio;

    raylib::Camera camera({0, 120, 500}, {0, 0, 0});  
    
    raylib::Texture snow("textures/snow.jpg");
    raylib::Model ground = raylib::Mesh::Plane(10000, 10000, 50, 50, 25).LoadModelFrom();
    ground.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = snow;

    raylib::Model penguin("models/penguin.glb");
    penguin.transform = raylib::Transform(penguin.transform).Scale(30);

    raylib::Model eagle("models/eagle.glb");
    eagle.transform = raylib::Transform(eagle.transform).Scale(10).RotateY(raylib::Degree(-90));

    cs381::SkyBox skybox("textures/skybox.png");

    const float acceleration = 10;
    std::vector<Entity> allEntities;
    allEntities.reserve(3);

    allEntities.emplace_back(raylib::Vector3{-50, 0, 0}, 0, 0, 0, &penguin);
    allEntities.emplace_back(raylib::Vector3{50, 0, 0}, 0, 0, 0, &penguin);
    allEntities.emplace_back(raylib::Vector3{0, 50, 0}, 0, 0, 0, &eagle, true);

    bool hold_mode = false;
    bool wait = false;

    raylib::Vector3 velocity = {0, 0, 0};

    int selected = 0;

    while(!window.ShouldClose())
    {
        window.BeginDrawing();
        {
            if(raylib::Keyboard::IsKeyPressed(KEY_TAB))
            {
                selected = (selected + 1) % allEntities.size();
            }
            if(!hold_mode)
                {
                    if(raylib::Keyboard::IsKeyPressed(KEY_W))
                    {
                        allEntities[selected].target_speed += 10;
                    }
                    if(raylib::Keyboard::IsKeyPressed(KEY_S))
                    {
                        allEntities[selected].target_speed -= 10;
                    }
                    if(raylib::Keyboard::IsKeyPressed(KEY_A))
                    {
                        allEntities[selected].heading -= 10;
                    }
                    if(raylib::Keyboard::IsKeyPressed(KEY_D))
                    {
                        allEntities[selected].heading += 10;
                    }
                }
            else
                {
                    if(raylib::Keyboard::IsKeyDown(KEY_W))
                    {
                        allEntities[selected].target_speed += 1;
                    }
                    if(raylib::Keyboard::IsKeyDown(KEY_S))
                    {
                        allEntities[selected].target_speed -= 1;
                    }
                    if(raylib::Keyboard::IsKeyDown(KEY_A))
                    {
                        allEntities[selected].heading -= 1;
                    }
                    if(raylib::Keyboard::IsKeyDown(KEY_D))
                    {
                        allEntities[selected].heading += 1;
                    }
                }
                if(raylib::Keyboard::IsKeyPressed(KEY_LEFT_SHIFT))
                {
                    allEntities[selected].target_speed = 0;
                }
                if(raylib::Keyboard::IsKeyPressed(KEY_RIGHT_SHIFT))
                {
                    allEntities[selected].position.SetX(0);
                    if(allEntities[selected].isEagle)
                    {
                        allEntities[selected].position.SetY(50);  
                    }
                    allEntities[selected].position.SetZ(0);
                }
                if(raylib::Keyboard::IsKeyPressed(KEY_SPACE))
                {   
                    allEntities[selected].speed = 0;
                    allEntities[selected].target_speed = 0;
                    velocity = 0;
                }

                allEntities[selected].heading = angle_clamp(allEntities[selected].heading);

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

            for(auto &e: allEntities)
            {
                if(e.speed < e.target_speed)
                {
                    e.speed += acceleration * dt;
                    //std::cout << "Acc applied" << std::endl;
                    std::cout << e.target_speed << std::endl << e.speed << std::endl;
                }
                else if (e.speed > e.target_speed)
                {
                    e.speed -= acceleration * dt;
                }

                if(e.isEagle)
                {
                    velocity = raylib::Vector3(-cos(e.heading.RadianValue()), -sin(e.heading.RadianValue()), 0) * e.speed;
                }
                else
                {
                    velocity = raylib::Vector3(-cos(e.heading.RadianValue()), 0, sin(e.heading.RadianValue())) * e.speed;
                }
                

                e.position += velocity * dt;

                // Wrapping
                if(e.position.x > 350)
                {
                    e.position.x = -350;
                }
                if(e.position.x < -350)
                {
                    e.position.x = 350;
                }
                if(e.position.z > 500)
                {
                    e.position.z = -500;
                }
                if(e.position.z < -500)
                {
                    e.position.z = 500;
                }
                if(e.position.y < 20 && e.isEagle)
                {
                    e.position.y = 20;
                }
            }

            camera.BeginMode();
            {
                skybox.Draw();
                ground.Draw({});
                for(size_t i = 0; i < allEntities.size(); i++)
                {
                    auto& e = allEntities[i];

                    if(e.isEagle)
                    {
                        DrawBoundedModel(*e.model, i == selected,[e](raylib::Transform& transform) -> raylib::Transform
                        {
                            return transform.Translate(e.position).RotateZ(raylib::Degree(e.heading));
                        });
                    }
                    else
                    {
                        DrawBoundedModel(*e.model, i == selected,[e](raylib::Transform& transform) -> raylib::Transform
                        {
                            return transform.Translate(e.position).RotateY(raylib::Degree(e.heading));
                        }); 
                    }
                    
                }
            }
            camera.EndMode();
        }
        window.EndDrawing();
    }
}