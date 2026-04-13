#include "AudioDevice.hpp"
#include "Camera3D.hpp"
#include "Keyboard.hpp"
#include "Matrix.hpp"
#include "Model.hpp"
#include "RadiansDegrees.hpp"
#include "Vector3.hpp"
#include "Vector4.hpp"
#include "raylib-cpp.hpp"
#include "raylib.h"
#include <concepts>
#include <execution>
#include <iostream>
#include <limits>
#include <optional>
#include <memory>
#include <vector>
#define SKYBOX_IMPLEMENTATION
#include "skybox.hpp"
#include <iostream>

size_t globalComponentCounter = 0;
template<typename T>
size_t GetComponentID(/* T reference = {} */) {
    static size_t id = globalComponentCounter++;
    return id;
}

raylib::Degree angle_normalize(raylib::Degree angle) {
    float decimal = float(angle) - int(angle);
    int whole = int(angle) % 360; // [-360-360]
    whole += (angle < 0) * 360; // [0-360]
    return decimal + whole;
}

using entity = size_t;
entity selected = 0;

struct ComponentStorageBase {
    virtual ~ComponentStorageBase() {}
    virtual size_t element_size() { return 0; }

    virtual void* Get(size_t index) = 0;
    virtual void* GetOrAllocate(size_t index) = 0;
};

template<typename Tcomponent>
struct ComponentStorage: public ComponentStorageBase, std::vector<Tcomponent> {
    using std::vector<Tcomponent>::vector;

    size_t element_size() override { return sizeof(Tcomponent); }

    void* Get(size_t index) override { 
        return &this->at(index);
    }

    void* GetOrAllocate(size_t index) override {
        if(this->size() <= index)
            this->resize(index + 1);
        return Get(index);
    }
};

struct Context {
    std::vector<std::vector<bool>> entityMasks;
    std::vector<std::shared_ptr<ComponentStorageBase>> storages = {nullptr};

    template<typename Tcomponent>
    ComponentStorageBase& GetStorage() {
        size_t id = GetComponentID<Tcomponent>();
        if(storages.size() <= id)
            storages.insert(
                storages.end(), 
                std::max<int64_t>(id - storages.size(), 1), 
                nullptr
            );
        if(!storages[id] || storages[id]->element_size() == 0)
            storages[id] = std::make_shared<ComponentStorage<Tcomponent>>();
        return *storages[id];
    }

    entity CreateEntity() {
        entity e = entityMasks.size();
        entityMasks.emplace_back(std::vector<bool>{false});
        return e;
    }

    // EC Homework: how do we remove entities?

    template<typename Tcomponent>
    Tcomponent& AddComponent(entity e) {
        size_t id = GetComponentID<Tcomponent>();
        auto& mask = entityMasks[e];
        if(mask.size() <= id)
            mask.resize(id + 1, false);
        mask[id] = true;
        return *(Tcomponent*)GetStorage<Tcomponent>().GetOrAllocate(e);
    }

    template<typename Tcomponent>
    Tcomponent& GetComponent(entity e) {
        size_t id = GetComponentID<Tcomponent>();
        assert(HasComponent<Tcomponent>(e));
        return *(Tcomponent*)GetStorage<Tcomponent>().Get(e);
    }

    template<typename Tcomponent>
    bool HasComponent(entity e) {
        size_t id = GetComponentID<Tcomponent>();
        return entityMasks.size() > e && entityMasks[e].size() > id && entityMasks[e][id];
    }
};

/***************************************************************
**
**      BEGIN COMPONENTS
**
****************************************************************/

struct SelectedComponent
{
    bool selected;
};

struct ModelComponent {
    raylib::Model* model;
};

struct PositionComponent {
    raylib::Vector3 position;
};

struct PhysicsComponent
{
    raylib::Vector3 velocity;
    float speed;
    float target_speed;
    float acceleration;
    float target_speed_mod;
};

struct PhysicsComponent2D
{
    raylib::Degree heading;
    float heading_mod;
};

struct PhysicsComponent3D
{
    raylib::Quaternion rotation;
};

/***************************************************************
**
**      BEGIN SYSTEMS
**
****************************************************************/

void DrawModelSystem(Context& ctx) {
    for(entity e = 0; e < ctx.entityMasks.size(); ++e)
    {
        if(!ctx.HasComponent<SelectedComponent>(e)) continue;
        if(!ctx.HasComponent<ModelComponent>(e)) continue;
        if(!ctx.HasComponent<PositionComponent>(e)) continue;


        auto model = ctx.GetComponent<ModelComponent>(e);
        auto position = ctx.GetComponent<PositionComponent>(e);
        auto selected = ctx.GetComponent<SelectedComponent>(e);

        auto backupTransform = model.model->transform;
        if(ctx.HasComponent<PhysicsComponent3D>(e))
        {
            // Holy shit what a bullshit ass line of code
            model.model->transform = raylib::Transform(model.model->transform).Translate(position.position).RotateXYZ(ctx.GetComponent<PhysicsComponent3D>(e).rotation.ToEuler());
        }
        else if(ctx.HasComponent<PhysicsComponent2D>(e))
        {
            model.model->transform = raylib::Transform(model.model->transform).Translate(position.position).RotateY(ctx.GetComponent<PhysicsComponent2D>(e).heading);
        }
        
        model.model->Draw({});  

        if(selected.selected)
        {
            DrawBoundingBox(model.model->GetTransformedBoundingBox(), BLUE);
        }

        model.model->transform = backupTransform;
    }
}

void ComputePhysicsSystem(Context &ctx, float dt)
{
    for(entity e = 0; e < ctx.entityMasks.size(); ++e)
    {
        if(!ctx.HasComponent<PositionComponent>(e)) continue;
        if(!ctx.HasComponent<PhysicsComponent>(e)) continue;

        auto &position = ctx.GetComponent<PositionComponent>(e);
        auto &physics = ctx.GetComponent<PhysicsComponent>(e);

        position.position += physics.velocity * dt;
    }
}

void Compute2DPhysicsSystem(Context &ctx, float dt)
{
    for(entity e = 0; e < ctx.entityMasks.size(); ++e)
    {
        if(!ctx.HasComponent<PhysicsComponent2D>(e)) continue;
        if(!ctx.HasComponent<PhysicsComponent>(e)) continue;

        auto &physics2d = ctx.GetComponent<PhysicsComponent2D>(e);
        auto &physics = ctx.GetComponent<PhysicsComponent>(e);

        if(physics.speed < physics.target_speed)
        {
            physics.speed += physics.acceleration * dt;
        }
        else if (physics.speed > physics.target_speed)
        {
            physics.speed -= physics.acceleration * dt;
        }
            
        physics.velocity = raylib::Vector3(-cos(physics2d.heading.RadianValue()), 0, sin(physics2d.heading.RadianValue())) * physics.speed;   
    }
}

void Compute3DPhysicsSystem(Context &ctx, float dt)
{
    for(entity e = 0; e < ctx.entityMasks.size(); ++e)
    {
        if(!ctx.HasComponent<PhysicsComponent3D>(e)) continue;
        if(!ctx.HasComponent<PhysicsComponent>(e)) continue;

        auto physics3d = ctx.GetComponent<PhysicsComponent3D>(e);
        auto physics = ctx.GetComponent<PhysicsComponent>(e);

        if(physics.speed < physics.target_speed)
        {
            physics.speed += physics.acceleration * dt;
        }
        else if (physics.speed > physics.target_speed)
        {
            physics.speed -= physics.acceleration * dt;
        }
        raylib::Vector3 tempvel = {1, 0, 0};
        physics.velocity = tempvel.RotateByQuaternion(physics3d.rotation) * physics.speed;
    }
}

void InputSystem(Context &ctx)
{
    for(entity e = 0; e < ctx.entityMasks.size(); ++e)
    {
        if(!ctx.HasComponent<SelectedComponent>(e)) continue;
        if(!ctx.GetComponent<SelectedComponent>(e).selected) continue;
        if(!ctx.HasComponent<PhysicsComponent>(e)) continue;

        auto &physics = ctx.GetComponent<PhysicsComponent>(e);
        if(raylib::Keyboard::IsKeyPressed(KEY_W))
        {
            physics.target_speed += physics.target_speed_mod;
        }
        else if(raylib::Keyboard::IsKeyPressed(KEY_S))
        {
            physics.target_speed -= physics.target_speed_mod;
        }
        else if(raylib::Keyboard::IsKeyPressed(KEY_SPACE))
        {
            physics.target_speed = 0;
        }

        // Eagle Specific 

        // TODO: FIX "EASING" ROTATION MOTION
        if(ctx.HasComponent<PhysicsComponent3D>(e))
        {
            auto &physics3d = ctx.GetComponent<PhysicsComponent3D>(e);
            if(raylib::Keyboard::IsKeyDown(KEY_R))
            {
                //physics3d.rotation.SetZ(physics3d.rotation.GetZ() + 0.1);
                physics3d.rotation.z += 0.1;
            }
            else if(raylib::Keyboard::IsKeyDown(KEY_F))
            {
                //physics3d.rotation.SetZ(physics3d.rotation.GetZ() - 0.1);
                physics3d.rotation.z -= 0.1;
            } 
            if(raylib::Keyboard::IsKeyDown(KEY_Q))
            {
                // physics3d.rotation.SetX(physics3d.rotation.GetX() + 0.1);
                physics3d.rotation.x += 0.1;
            }
            else if(raylib::Keyboard::IsKeyDown(KEY_E))
            {
                // physics3d.rotation.SetX(physics3d.rotation.GetX() - 0.1);
                physics3d.rotation.x -= 0.1;
            }
            if(raylib::Keyboard::IsKeyDown(KEY_D))
            {
                // physics3d.rotation.SetY(physics3d.rotation.GetX() - 0.1);
                physics3d.rotation.y += 0.1;
            }
            else if(raylib::Keyboard::IsKeyDown(KEY_A))
            {
                // physics3d.rotation.SetY(physics3d.rotation.GetX() + 0.1);
                physics3d.rotation.y -= 0.1;
            }
        }
        else
        {
            auto &physics2D = ctx.GetComponent<PhysicsComponent2D>(e);
            if(raylib::Keyboard::IsKeyDown(KEY_A))
            {
                physics2D.heading += physics2D.heading_mod;
            }
            else if(raylib::Keyboard::IsKeyDown(KEY_D))
            {
                physics2D.heading -= physics2D.heading_mod;
            }
        }
    }   
}

/***************************************************************
**
**      BEGIN MAIN
**
****************************************************************/

int main() {
    raylib::Window window(800, 600, "As0");
    window.SetState(FLAG_WINDOW_RESIZABLE);
    raylib::AudioDevice audio;

    raylib::Model penguin("models/penguin.glb");
    penguin.transform = raylib::Transform(penguin.transform).Scale(30);
    raylib::Camera camera({0, 120, 500}, {0, 0, 0});
    raylib::Model eagle("models/eagle.glb");
    eagle.transform = raylib::Transform(eagle.transform).Scale(10);
    raylib::Model ground = raylib::Mesh::Plane(10000, 10000, 50, 50, 25).LoadModelFrom();
    raylib::Texture snow("textures/snow.jpg");
    ground.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = snow;

    cs381::SkyBox skybox("textures/skybox.png");
    
    Context ctx;

    // Entity Setup
    for(int i = 0; i < 10; i++)
    {
        auto e = ctx.CreateEntity();
        if(i < 5)
        {
            ctx.AddComponent<ModelComponent>(e).model = &penguin;
            ctx.AddComponent<PositionComponent>(e).position = raylib::Vector3{-200 + 100*i, 0, 0};
            ctx.AddComponent<PhysicsComponent>(e).acceleration = 2 * (i + 1);
            ctx.GetComponent<PhysicsComponent>(e).target_speed_mod = 1 * (i + 1);
            ctx.AddComponent<PhysicsComponent2D>(e).heading_mod = 1 * (i + 1);
            ctx.GetComponent<PhysicsComponent2D>(e).heading = 90;
            ctx.AddComponent<SelectedComponent>(e).selected = false;
        }
        else
        {
            ctx.AddComponent<ModelComponent>(e).model = &eagle;
            ctx.AddComponent<PositionComponent>(e).position = raylib::Vector3{-200 + 100 * (i - 5), 150, 0};
            ctx.AddComponent<PhysicsComponent>(e).acceleration = 5;
            ctx.AddComponent<PhysicsComponent3D>(e);
            ctx.AddComponent<SelectedComponent>(e).selected = false;
        }
    }
    ctx.GetComponent<SelectedComponent>(selected).selected = true;
    
    while(!window.ShouldClose()) 
    {
        window.BeginDrawing(); 
        {
            if(raylib::Keyboard::IsKeyPressed(KEY_TAB))
            {
                ctx.GetComponent<SelectedComponent>(selected).selected = false;
                selected = (selected + 1) % ctx.entityMasks.size();
                ctx.GetComponent<SelectedComponent>(selected).selected = true;
            }
            window.ClearBackground(raylib::Color::RayWhite());
            float dt = window.GetFrameTime();
        
            camera.BeginMode(); {
                skybox.Draw();
                ground.Draw({});
                InputSystem(ctx);
                Compute3DPhysicsSystem(ctx, dt);
                Compute2DPhysicsSystem(ctx, dt);
                ComputePhysicsSystem(ctx, dt);
                DrawModelSystem(ctx);
            } camera.EndMode();
            
            window.DrawFPS();
        } window.EndDrawing();
    }
}