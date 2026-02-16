#include "raylib-cpp.hpp"
#include "raylib.h"

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

int main()
{
    raylib::Window window(800, 600, "AS 2");
    window.SetState(FLAG_WINDOW_RESIZABLE);

    raylib::Model penguin("models/penguin.glb");
    penguin.transform = raylib::Transform(penguin.transform).Scale(30);
    raylib::Model eagle("models/eagle.glb");
    eagle.transform = raylib::Transform(eagle.transform).Scale(10);

    // Position: Where the camera is
    // Target: Where the camera is looking

    raylib::Camera camera({0, 120, 500}, {0, 0, 0,});

    while(!window.ShouldClose())
    {
        window.BeginDrawing();
        window.ClearBackground(raylib::Color::Gray());
        camera.BeginMode();
            {
                // Penguin 1
                DrawBoundedModel(penguin, [](raylib::Transform& transform) -> raylib::Transform
                {
                    return transform.Translate({-200, 0, 0}); // .RotateX(raylib::Degree(90));
                });
                // Penguin 2
                DrawBoundedModel(penguin, [](raylib::Transform& transform) -> raylib::Transform
                {
                    return transform.Translate({200, 0, 0}).RotateY(raylib::Degree(90)); // .RotateX(raylib::Degree(90));
                });
                // Penguin 3
                DrawBoundedModel(penguin, [](raylib::Transform& transform) -> raylib::Transform
                {
                    return transform.Translate({100, 100, 0}).Scale(1, 2, 1).RotateY(raylib::Degree(270)); // .RotateX(raylib::Degree(90));
                });
                // Eagle 1
                DrawBoundedModel(eagle, [](raylib::Transform& transform) -> raylib::Transform
                {
                    return transform.Translate({0, 0, 0}).Scale(1, 1, 1); // .RotateX(raylib::Degree(90));
                });
                // Eagle 2
                DrawBoundedModel(eagle, [](raylib::Transform& transform) -> raylib::Transform
                {
                    return transform.Translate({-100, 100, 0}).Scale(1, -1, 1).RotateY(raylib::Degree(180)); // .RotateX(raylib::Degree(90));
                });
            }
        camera.EndMode();
        
        window.EndDrawing();
    }
}