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

void DrawModelSystem(Context& ctx) {
    for(entity e = 0; e < ctx.entityMasks.size(); ++e)
    {
        if(!ctx.HasComponent<ModelComponent>(e)) continue;
        if(!ctx.HasComponent<PositionComponent>(e)) continue;

        auto model = ctx.GetComponent<ModelComponent>(e);
        auto position = ctx.GetComponent<PositionComponent>(e);
        model.model->Draw(position.position);  
    }
}

void ComputePhysicsSystem(Context &ctx)
{
    for(entity e = 0; e < ctx.entityMasks.size(); ++e)
    {
        if(!ctx.HasComponent<PositionComponent>(e)) continue;
        if(!ctx.HasComponent<PhysicsComponent>(e)) continue;

        auto position = ctx.GetComponent<PositionComponent>(e);
        auto physics = ctx.GetComponent<PhysicsComponent>(e);

        if(physics.speed < physics.target_speed)
        {
            
        }
        else
        {

        }
    }
}

void Compute2DPhysicsSystem(Context &ctx)
{
    for(entity e = 0; e < ctx.entityMasks.size(); ++e)
    {
        if(!ctx.HasComponent<PhysicsComponent2D>(e)) continue;
        if(!ctx.HasComponent<PhysicsComponent>(e)) continue;

        auto physics2d = ctx.GetComponent<PhysicsComponent2D>(e);
        auto physics = ctx.GetComponent<PhysicsComponent>(e);

        
    }
}

void Compute3DPhysicsSystem(Context &ctx)
{
    for(entity e = 0; e < ctx.entityMasks.size(); ++e)
    {
        if(!ctx.HasComponent<PhysicsComponent3D>(e)) continue;
        if(!ctx.HasComponent<PhysicsComponent>(e)) continue;

        auto physics3d = ctx.GetComponent<PhysicsComponent3D>(e);
        auto physics = ctx.GetComponent<PhysicsComponent>(e);

        
    }
}

/*
// auto sequential(auto func) {
//     return [func](Context& ctx) {
//         // Bulk process
//         for(entity e = 0; e < ctx.entityMasks.size(); ++e) {
//             func(ctx, e);
//         }
//     };
// }

// auto parallel(auto func) {
//     return [func](Context& ctx) {
//         std::vector<entity> entities(ctx.entityMasks.size());
//         std::iota(entities.begin(), entities.end(), 0);
//         std::for_each(std::execution::par_unseq, entities.begin(), entities.end(), [func, &ctx](entity e){
//             func(ctx, e);
//         });
//     };
// }
*/

// template<std::invocable<Context&>... Tsystems>
// auto sequential(Tsystems... systems) {
//     return [=](Context& ctx) {
//         (systems(ctx), ...);
//     };
// }

int main() {
    raylib::Window window(800, 600, "As0");
    window.SetState(FLAG_WINDOW_RESIZABLE);
    raylib::AudioDevice audio;

    raylib::Model penguin("models/penguin.glb");
    penguin.transform = raylib::Transform(penguin.transform).Scale(30).RotateY(raylib::Degree(90));
    raylib::Camera camera({0, 120, 500}, {0, 0, 0});

    raylib::Model ground = raylib::Mesh::Plane(10000, 10000, 50, 50, 25).LoadModelFrom();
    raylib::Texture snow("textures/snow.jpg");
    ground.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = snow;

    cs381::SkyBox skybox("textures/skybox.png");

    const float acceleration = 10;
    
    Context ctx;
    auto e = ctx.CreateEntity();
    ctx.AddComponent<ModelComponent>(e).model = &penguin;
    ctx.AddComponent<PositionComponent>(e).position = raylib::Vector3{0, 0, 0};

    while(!window.ShouldClose()) {
        window.BeginDrawing(); {
            window.ClearBackground(raylib::Color::RayWhite());
            float dt = window.GetFrameTime();
        
            camera.BeginMode(); {
                skybox.Draw();
                ground.Draw({});

                DrawModelSystem(ctx);

            } camera.EndMode();
            
            window.DrawFPS();
        } window.EndDrawing();
    }
}