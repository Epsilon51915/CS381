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

#define SKYBOX_IMPLEMENTATION

#include "skybox.hpp"

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
    float speed = 0;
    float target_speed = 0;
    raylib::Degree heading = 0;
    raylib::Vector3 position = {0, 0, 0};
    raylib::Vector3 velocity = {0, 0, 0};
    float acceleration = 0;

    void Setup() override{};
    void Update(float dt) override
    {
        if(auto t = attatched->GetComponent<TransformComponent>(); t)
        {
            // Smoothly glide speed to target speed
            if(speed < target_speed)
            {
                speed += acceleration * dt;
            }
            else if (speed > target_speed)
            {
                speed -= acceleration * dt;
            }

            // Modify entity velocity according to heading and speed
            heading = angle_clamp(heading);
            if(attatched->isEagle)
            {
                velocity = raylib::Vector3(-cos(heading.RadianValue()), -sin(heading.RadianValue()), 0) * speed;
            }
            else
            {
                velocity = raylib::Vector3(-cos(heading.RadianValue()), 0, -sin(heading.RadianValue())) * speed;
            }
            
            auto& transform = t->get();
            transform.position += velocity * dt;

            // NOTE: Quaternion rotations are based off of RADIAN values, while RotateXYZ operate with DEGREE values. Keep this in mind when trying to rotate objects using one method or another
            if(attatched->isEagle)
            {
                transform.rotation = transform.rotation.FromEuler({0, 0, raylib::Radian(heading)});
            }
            else
            {
                transform.rotation = transform.rotation.FromEuler({0, -raylib::Radian(heading), 0});
            }

            // Wrapping
            if(transform.position.x > 350)
            {
                transform.position.x = -350;
            }
            if(transform.position.x < -350)
            {
                transform.position.x = 350;
            }
            if(transform.position.z > 500)
            {
                transform.position.z = -500;
            }
            if(transform.position.z < -500)
            {
                transform.position.z = 500;
            }
            if(transform.position.y < 20 && attatched->isEagle)
            {
                transform.position.y = 20;
            }
        }
    };
    void Cleanup() override{};
};

// Processes input for each entity
struct InputComponent : public Component
{
    float targetSpeedMod = 0;
    float headingMod = 0;
    void Setup() override{};
    void Update(float dt) override
    {
        /*if(auto t = attatched->GetComponent<PhysicsComponent>(); t)
        {
            auto& physics = t->get();
            if(attatched->isSelected)
            {
                // General movement
                if(raylib::Keyboard::IsKeyDown(KEY_W))
                {
                    physics.target_speed += targetSpeedMod;
                }
                if(raylib::Keyboard::IsKeyDown(KEY_S))
                {
                    physics.target_speed -= targetSpeedMod;
                }
                if(raylib::Keyboard::IsKeyDown(KEY_A))
                {
                    physics.heading += headingMod;
                }
                if(raylib::Keyboard::IsKeyDown(KEY_D))
                {
                    physics.heading -= headingMod;
                }

                // Sets target speed to 0, has entity slow to a stop
                if(raylib::Keyboard::IsKeyPressed(KEY_SPACE))
                {
                    physics.target_speed = 0;
                }

                // Sets position back to the origin (0, 0, 0) for penguins, (0, 50, 0) for eagles
                if(raylib::Keyboard::IsKeyPressed(KEY_RIGHT_SHIFT))
                {
                    physics.position.SetX(0);
                    if(attatched->isEagle)
                    {
                        physics.position.SetY(50);  
                    }
                    physics.position.SetZ(0);
                }
                
                // Sets target speed, speed, and velocity to 0, stopping entity instantly
                if(raylib::Keyboard::IsKeyPressed(KEY_LEFT_SHIFT))
                {   
                    physics.speed = 0;
                    physics.target_speed = 0;
                    physics.velocity = 0;
                }
            }
    
        }*/
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
    raylib::Window window(800, 600, "AS5");
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
    /*
        =====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====
        CHANGE MAY BEGIN FROM HERE DOWNWARDS
        =====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====
    */

    const float acceleration = 10;
    std::vector<Entity> allEntities;
    allEntities.reserve(10);


    for(int i = 0; i < 10; i++)
    {
        auto& e = allEntities.emplace_back();
        auto& transform = e.AddComponent<TransformComponent>();
        auto& model = e.AddComponent<DrawModelComponent>();
        auto& physics = e.AddComponent<PhysicsComponent>();
        auto& input = e.AddComponent<InputComponent>();

        if(i < 5)
        {
            e.isEagle = false;
            model.model = &penguin;
            transform.position = raylib::Vector3{-200 + (float)i * 100, 0, 0};
            input.headingMod = 10 * (i+1);
            input.targetSpeedMod = 10 * (i+1);
            physics.acceleration = 10 * (i+1);
            physics.position = raylib::Vector3{-200 + (float)i * 100, 0, 0};
        }
        else
        {
            e.isEagle = true;
            model.model = &eagle;
            transform.position = raylib::Vector3{-200 + (float)(i-5) * 100, 150, 0};
            input.headingMod = .1;
            input.targetSpeedMod = 10;
            physics.acceleration = 10;
            physics.position = raylib::Vector3{-200 + (float)(i-5) * 100, 150, 0};
        }
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
    input["forward"] = raylib::Action::key(KEY_W).AddPressedCallback([&allEntities, &selected]()
    {
        auto& input = allEntities[selected].GetComponent<InputComponent>()->get();
        auto& physics = allEntities[selected].GetComponent<PhysicsComponent>()->get();
        physics.target_speed += input.targetSpeedMod;
    }).move();
    input["back"] = raylib::Action::key(KEY_S).AddPressedCallback([&allEntities, &selected]()
    {
        auto& input = allEntities[selected].GetComponent<InputComponent>()->get();
        auto& physics = allEntities[selected].GetComponent<PhysicsComponent>()->get();
        physics.target_speed -= input.targetSpeedMod;
    }).move();
    input["left"] = raylib::Action::key(KEY_A).AddPressedCallback([&allEntities, &selected]()
    {
        auto& input = allEntities[selected].GetComponent<InputComponent>()->get();
        auto& physics = allEntities[selected].GetComponent<PhysicsComponent>()->get();
        physics.heading += input.headingMod;
    }).move();
    input["right"] = raylib::Action::key(KEY_D).AddPressedCallback([&allEntities, &selected]()
    {
        auto& input = allEntities[selected].GetComponent<InputComponent>()->get();
        auto& physics = allEntities[selected].GetComponent<PhysicsComponent>()->get();
        physics.heading -= input.headingMod;
    }).move();
    input["slowstop"] = raylib::Action::key(KEY_SPACE).AddPressedCallback([&allEntities, &selected]()
    {
        auto& physics = allEntities[selected].GetComponent<PhysicsComponent>()->get();
        physics.target_speed = 0;
    }).move();
    input["hardstop"] = raylib::Action::key(KEY_LEFT_SHIFT).AddPressedCallback([&allEntities, &selected]()
    {
        auto& physics = allEntities[selected].GetComponent<PhysicsComponent>()->get();
        physics.target_speed = 0;
        physics.speed = 0;
        physics.velocity = 0;
    }).move();
    input["resetposition"] = raylib::Action::key(KEY_RIGHT_SHIFT).AddPressedCallback([&allEntities, &selected]()
    {
        auto& physics = allEntities[selected].GetComponent<PhysicsComponent>()->get();
        auto& transform = allEntities[selected].GetComponent<TransformComponent>()->get();
        physics.position.SetX(0);
        transform.position.SetX(0);
        if(allEntities[selected].isEagle)
        {
            physics.position.SetY(150);
            transform.position.SetY(150);
        }
        else
        {
            physics.position.SetZ(0);
            transform.position.SetZ(0);
        }
    }).move();

    while(!window.ShouldClose())
    {
        input.PollEvents();
        window.BeginDrawing();
        {
            window.ClearBackground(raylib::Color::RayWhite());
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
        }
        window.EndDrawing();   
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

/*


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
                return transform.Translate(e.position).c;
            }); 
        }
    }
                    
*/