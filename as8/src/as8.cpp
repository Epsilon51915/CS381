#include "raylib-cpp.hpp"
#include "raylib.h"
#include "AudioDevice.hpp"
#include "Camera3D.hpp"
#include "Matrix.hpp"
#include "Model.hpp"
#include "RadiansDegrees.hpp"
#include <iostream>
#include <memory>

#define SKYBOX_IMPLEMENTATION

#include "skybox.hpp"

extern size_t globalComponentCounter;

template<typename T>
size_t GetComponentId(T reference = {})
{
    static size_t id = globalComponentCounter++;
    return id;
}

using entity = size_t;

struct ComponentStorageBase
{
    virtual ~ComponentStorageBase() {};
    virtual size_t element_size() {return 0;}

    virtual void* Get(size_t index) = 0;
    virtual void* GetOrAllocate(size_t index) = 0;
};

template<typename Tcomponent>
struct ComponentStorage : public ComponentStorageBase, std::vector<Tcomponent>
{
    using std::vector<Tcomponent>::vector;

    size_t element_size() override {return sizeof(Tcomponent);}

    void* Get(size_t index) override
    {
        return &this->at(index);
    }

    void* GetOrAllocate(size_t index) override
    {
        if(this->size() <= index)
        {
            this->resize(index + 1);
        }
        return Get(index);
    }
};

struct Context
{
    std::vector<std::vector<bool>> entityMasks;
    std::vector<std::shared_ptr<ComponentStorageBase>> storages = {nullptr};

    template<typename Tcomponent>
    ComponentStorageBase& GetStorage()
    {
        size_t id = GetComponentId<Tcomponent>();
        if(storages.size() <= id)
        {
            storages.insert(storages.end(), std::max<int64_t>(id - storages.size(), 1), nullptr);
        }
        if(!storages[id] || storages[id]->element_size() == 0)
        {
            storages[id] = std::make_shared<ComponentStorage<Tcomponent>>();
        }
        return *storages[id];
    }

    entity CreateEntity()
    {
        entity e = entityMasks.size();
        entityMasks.emplace_back(std::vector<bool>{false});
        return e;
    }

    // EC HW: How do we remove entities?

    // Add component to entity e
    template<typename Tcomponent>
    Tcomponent& AddComponent(entity e)
    {
        size_t id = GetComponentId<Tcomponent>();
        auto &mask = entityMasks[e];
        if(mask.size() <= id)
        {
            mask.resize(id+1, false);
        }
        mask[id] = true;
        return *(Tcomponent*)GetStorage<Tcomponent>().GetOrAllocate(e);
    }

    // Get component from entity e
    template<typename Tcomponent>
    Tcomponent& GetComponent(entity e)
    {
        size_t id = GetComponentId<Tcomponent>();
        assert(HasComponent<Tcomponent>(e));
        return *(Tcomponent*)GetStorage<Tcomponent>().Get(e);
    }

    // Check if entity e has a component
    template<typename Tcomponent>
    Tcomponent& HasComponent(entity e)
    {
        size_t id = GetComponentId<Tcomponent>();
        return entityMasks.size() > e && entityMasks[e].size() > id && entityMasks[e][id];
    }
};

// Entities are indexes to arrays of components which are bulk processed by systems
// Systems are batch processes: Consider every entity in the context it is given
void Draw(Context& ctx)
{
    for(entity e = 0; e < ctx.entityMasks.size(); ++e)
    {
        if(ctx.HasComponent<DrawInfo>(e)) continue;

    }
}

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
    raylib::Window window(800, 600, "AS8");
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

    int selected = 0;

    while(!window.ShouldClose())
    {
        window.BeginDrawing();
        {
            // if(raylib::Keyboard::IsKeyPressed(KEY_TAB))
            // {
            //     selected = (selected + 1) % allEntities.size();
            // }
            

            float dt = window.GetFrameTime();
            window.ClearBackground(raylib::Color::RayWhite());

            camera.BeginMode();
            {
                skybox.Draw();
                ground.Draw({});
            }
            camera.EndMode();
        }
        window.EndDrawing();
    }
}