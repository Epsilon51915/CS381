#include "raylib-cpp.hpp"
#include <BufferedRaylib.hpp>
#include "raylib.h"
#include "AudioDevice.hpp"
#include "Camera3D.hpp"
#include "Matrix.hpp"
#include "Model.hpp"
#include "RadiansDegrees.hpp"
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <cstdlib>
#include <ctime>

#define GUI_LAYOUT_NAME_IMPLEMENTATION
#include "gui_layout_name.h"
#define SKYBOX_IMPLEMENTATION
#include "skybox.hpp"

bool game_over = false;
raylib::Degree angle_clamp(raylib::Degree angle)
{
    float decimal = float(angle) - int(angle);
    int whole = int(angle) % 360;
    whole += (angle < 0) * 360;
    return decimal + whole;
}

// A component is something that defines some behaviour for an entity
class Component
{
    public:
        struct Entity* attatched;

        virtual void Setup() = 0;
        virtual void Update(float dt) = 0;
        virtual void Cleanup() = 0;
};

// An entity is a list of components
struct Entity
{
    std::vector<std::shared_ptr<Component>> components;

    // Adds a component + tells component what entity is attatched to it
    template<std::derived_from<Component> T>
    T& AddComponent()
    {
        auto out = components.emplace_back(std::make_shared<T>());
        out->attatched = this;
        return (T&)*out;
    } 

    // Retrieves component or nothing if no component is attatched
    template<std::derived_from<Component> T>
    std::optional<std::reference_wrapper<T>> GetComponent()
    {
        for(auto& component : components)
        {
            T* cast = dynamic_cast<T*>(component.get());
            if(cast) return *cast;
        }

        return {};
    }
    bool isEagle = false;
    bool isSelected = false;
};

// Stores transform into for an entity, no behaviour
struct TransformComponent : public Component
{
    raylib::Vector3 position = {0, 0, 0};
    raylib::Quaternion rotation = raylib::Quaternion::Identity();

    void Setup() override{};
    void Update(float dt) override{};
    void Cleanup() override{};
};

// Draws model / Indicating what model it represents
struct DrawModelComponent : public Component
{
    raylib::Model* model;

    void Setup() override{};
    void Update(float dt) override
    {
        if(auto t = attatched->GetComponent<TransformComponent>(); t)
        {
            auto& transform = t->get();
            raylib::Transform backupTransform = model->transform;
            model->transform = raylib::Transform(model->transform).Translate(transform.position).Rotate(transform.rotation);
            model->Draw({});

            // Draw Bounding Box
            if(attatched->isSelected)
            {
                raylib::BoundingBox box = model->GetTransformedBoundingBox();
                DrawBoundingBox(box, raylib::Color::Blue());
            }
            model->transform = backupTransform;
        }
    };
    void Cleanup() override{};
};

// Processes physics for each entity
struct PhysicsComponent : public Component
{
    raylib::Degree heading = 0;
    raylib::Vector3 position = {0, 0, 0};
    float headingMod;
    float killTimer = 60;
    void Setup() override{};
    void Update(float dt) override
    {
        if(auto t = attatched->GetComponent<TransformComponent>(); t)
        {
            auto& transform = t->get();
            heading += headingMod;
            std::cout <<"Heading: " << heading << std::endl;
            heading = angle_clamp(heading);
            // NOTE: Quaternion rotations are based off of RADIAN values, while RotateXYZ operate with DEGREE values. Keep this in mind when trying to rotate objects using one method or another

            if(heading < 275 && heading > 265)
            {
                headingMod = 0;
                heading = 270;
                killTimer--;

                game_over = true;
            }
            if(killTimer == 0)
            {
                transform.position.y = -10;
                transform.position.z = 400;
            }
            if(attatched->isEagle)
            {
                transform.rotation = transform.rotation.FromEuler({0, 0, raylib::Radian(heading)});
            }
            else
            {
                transform.rotation = transform.rotation.FromEuler({0, -raylib::Radian(heading), 0});
            }
        }
    };
    void Cleanup() override{};
};

int main()
{
    /*
        =====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====
        DO NOT CHANGE: THIS IS ALL BASIC SETUP
        =====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====
    */
    std::srand(std::time(0));
    raylib::Window window(800, 600, "AS5");
    window.SetState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
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
    /*
        =====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====
        CHANGE MAY BEGIN FROM HERE DOWNWARDS
        =====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====
    */

    auto gui_state = InitGuiLayoutName();

    raylib::Music music("audio/scary.mp3");
    music.Play();
    raylib::Sound lightning_strike("audio/lightning.mp3");
    const float acceleration = 10;
    std::vector<Entity> allEntities;
    allEntities.reserve(1);
    float score = 0;
    float maxSpeed = 0.05;

    for(int i = 0; i < 1; i++)
    {
        auto& e = allEntities.emplace_back();
        auto& transform = e.AddComponent<TransformComponent>();
        auto& model = e.AddComponent<DrawModelComponent>();
        auto& physics = e.AddComponent<PhysicsComponent>();

        e.isEagle = false;
        model.model = &penguin;
        transform.position = raylib::Vector3{0, 0, 0};
        physics.position = raylib::Vector3{0, 0, 0};
        physics.headingMod = .05;
    }

    int selected = 0;

    // Setup
    for(Entity& e : allEntities)
    {
        for(std::shared_ptr<Component>& c : e.components)
        {
            c->Setup();
        }
    }

    allEntities.at(selected).isSelected = true;

    raylib::BufferedInput input;
    input["left"] = raylib::Action::key(KEY_A).AddPressedCallback([&allEntities, &selected, &maxSpeed]()
    {
        auto& physics = allEntities[selected].GetComponent<PhysicsComponent>()->get();
        physics.headingMod *= -1.15;
        if(abs(physics.headingMod) > maxSpeed)
        {
            maxSpeed = abs(physics.headingMod);
        }
    }).move();
    input["right"] = raylib::Action::key(KEY_D).AddPressedCallback([&allEntities, &selected, &maxSpeed]()
    {
        auto& physics = allEntities[selected].GetComponent<PhysicsComponent>()->get();
        physics.headingMod *= -1.15;
        if(abs(physics.headingMod) > maxSpeed)
        {
            maxSpeed = abs(physics.headingMod);
        }
    }).move();

    while(!window.ShouldClose())
    {
        input.PollEvents();
        window.BeginDrawing();
        {
            window.ClearBackground(raylib::Color::Black());
            float dt = window.GetFrameTime();
        
            // TAB: Changes selected entity
            if(raylib::Keyboard::IsKeyPressed(KEY_TAB))
            {
                allEntities.at(selected).isSelected = false;
                selected = (selected + 1) % allEntities.size();
                allEntities.at(selected).isSelected = true;
            }

            camera.BeginMode();
            {
                skybox.Draw();
                ground.Draw({});
                for(Entity& e : allEntities)
                {
                    for(std::shared_ptr<Component>& c : e.components)
                    {
                        c->Update(dt);
                    }
                }
            }
            camera.EndMode();

            //DrawFPS(500, 500);
            if(!game_over)
            {
                score += allEntities.size() * dt * maxSpeed;
            }
            std::string scre = "Score: " + std::to_string(int(score));
            //std::cout << scre << std::endl;
            DrawText(scre.c_str(), 500, 20, 20, WHITE);
        }
        GuiLayoutName(&gui_state);
        window.EndDrawing();   
        music.SetVolume(gui_state.Slider000Value/100);
        if(gui_state.Slider000Value != 0)
        {
            music.Update();
        }
        if(std::rand() % 500 == 0)
        {
            lightning_strike.Play();
        }
    }
        

    // Cleanup
    for(Entity& e : allEntities)
    {
        for(std::shared_ptr<Component>& c : e.components)
        {
            c->Cleanup();
        }
    }
}