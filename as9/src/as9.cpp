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

/***************************************************************
**
**      BEGIN COMPONENTS
**
****************************************************************/

struct Health
{
    float hp;
    float armor;
};

struct MapComponent
{
    char world[100][100];
    entity player;
};

struct PlayerLocations
{
    std::vector<int> all_posX;
    std::vector<int> all_posY;
};

struct PositionComponent
{
    int posX;
    int posY;
};

struct Inventory
{
    // Wood, Stone, Vine, Seeds
    int inventory[10];
    bool open;
};

/***************************************************************
**
**      BEGIN SYSTEMS
**
****************************************************************/

void SetupWorldSystem(Context &ctx)
{
    for(entity e = 0; e < ctx.entityMasks.size(); ++e)
    {
        if(!ctx.HasComponent<MapComponent>(e)) continue;

        auto &map = ctx.GetComponent<MapComponent>(e);
        for(int i = 0; i < 100; i++)
        {
            for(int j = 0; j < 100; j++)
            {
                if(i < 10 || i > 90)
                {
                    map.world[i][j] = '^';  
                }
                else if(j < 10 || j > 90)
                {
                    map.world[i][j] = '^';
                }
                else
                {
                    if(rand() % 25 == 0)
                    {
                        map.world[i][j] = ',';
                    }
                    else if(rand() % 25 == 0)
                    {
                        map.world[i][j] = 'o';
                    }
                    else if(rand() % 25 == 0)
                    {
                        map.world[i][j] = 'c';
                    }
                    else
                    {
                        map.world[i][j] = '.';
                    }   
                }
            }
        }
    }
}

void DrawWorldSystem(Context &ctx)
{
    for(entity e = 0; e < ctx.entityMasks.size(); ++e)
    {
        if(!ctx.HasComponent<MapComponent>(e)) continue;
        if(!ctx.HasComponent<PlayerLocations>(e)) continue;
        auto map = ctx.GetComponent<MapComponent>(e);
        auto player_locations = ctx.GetComponent<PlayerLocations>(e);

        int col, line;
        col = 1;
        for(int i = player_locations.all_posX.at(map.player) - 9; i < player_locations.all_posX.at(map.player) + 10; i++)
        {
            line = 1;
            for(int j = player_locations.all_posY.at(map.player) - 7; j < player_locations.all_posY.at(map.player) + 8; j++)
            {
                
                char buff[2] = {map.world[i][j], '\0'};
                if(map.world[i][j] == '^')
                {
                    DrawText(buff, 40 * col, 36 * line, 72, GREEN);
                }
                else if(i == player_locations.all_posX.at(map.player) && j == player_locations.all_posY.at(map.player))
                {
                    char player[2] = {'+', '\0'};
                    DrawText(player, 40 * col, 36 * (line+1), 36, RED);
                }
                else if(map.world[i][j] == '.')
                {
                    DrawText(buff, 40 * col, 36 * line, 72, WHITE);
                }
                else if(map.world[i][j] == ',')
                {
                    DrawText(buff, 40*col, 36*line, 72, BROWN);
                }
                else if(map.world[i][j] == 'o')
                {
                    DrawText(buff, 40*col, 36*(line+1), 24, GRAY);
                }
                else if(map.world[i][j] == 'x')
                {
                    DrawText(buff, 40*col, 36*(line+1), 24, BEIGE);
                }
                else if(map.world[i][j] == 'c')
                {
                    DrawText(buff, 40*col, 36*(line+1), 24, DARKGREEN);
                }
                
                line++;
            }
            col++;
        }
    }
}

void InputHandlerSystem(Context &ctx)
{
    for(entity e = 0; e < ctx.entityMasks.size(); ++e)
    {
        if(!ctx.HasComponent<PositionComponent>(e)) continue;
        if(!ctx.HasComponent<Inventory>(e)) continue;
        auto &inv = ctx.GetComponent<Inventory>(e);
        auto &position = ctx.GetComponent<PositionComponent>(e); 
        auto &world = ctx.GetComponent<PlayerLocations>(0);
        auto map = ctx.GetComponent<MapComponent>(0);
        if(raylib::Keyboard::IsKeyPressed(KEY_W))
        {
            if(map.world[position.posX][position.posY - 1] != '^')
            {
                world.all_posY.at(e)--;
                position.posY--;
            }
        }
        else if(raylib::Keyboard::IsKeyPressed(KEY_S))
        {
            if(map.world[position.posX][position.posY + 1] != '^')
            {
                world.all_posY.at(e)++;
                position.posY++;
            }
        }
        else if(raylib::Keyboard::IsKeyPressed(KEY_A))
        {
            if(map.world[position.posX-1][position.posY] != '^')
            {
                world.all_posX.at(e)--;
                position.posX--;
            }
        }
        else if(raylib::Keyboard::IsKeyPressed(KEY_D))
        {
            if(map.world[position.posX+1][position.posY] != '^')
            {
                world.all_posX.at(e)++;
                position.posX++;
            }
        }
        else if(raylib::Keyboard::IsKeyPressed(KEY_ENTER) && !inv.open)
        {
            inv.open = true;
        }
        else if(raylib::Keyboard::IsKeyPressed(KEY_ENTER) && inv.open)
        {
            inv.open = false;
        }
    }
    
}

void PickupItemSystem(Context &ctx)
{
    for(entity e = 0; e < ctx.entityMasks.size(); ++e)
    {
        if(!ctx.HasComponent<PositionComponent>(e)) continue;
        if(!ctx.HasComponent<Inventory>(e)) continue;  
        auto pos = ctx.GetComponent<PositionComponent>(e);
        auto &inv = ctx.GetComponent<Inventory>(e);
        auto &world = ctx.GetComponent<MapComponent>(0);

        if(world.world[pos.posX][pos.posY] == ',')
        {
            inv.inventory[0]++;
            world.world[pos.posX][pos.posY] = 'x';
            std::cout << "Picked up WOOD!" << std::endl;
        }
        else if(world.world[pos.posX][pos.posY] == 'o')
        {
            inv.inventory[1]++;
            world.world[pos.posX][pos.posY] = 'x';
            std::cout << "Picked up STONE!" << std::endl;
        }
        else if(world.world[pos.posX][pos.posY] == 'c')
        {
            if(rand() % 10 == 0)
            {
                inv.inventory[3]++;
                std::cout << "You found a SEED underneath the VINE!" << std::endl;
            }
            inv.inventory[2]++;
            world.world[pos.posX][pos.posY] = 'x';
            std::cout << "Picked up VINE!" << std::endl;
        }
    }
}

/***************************************************************
**
**      BEGIN MAIN
**
****************************************************************/
int main() {
    srand(time(nullptr));
    raylib::Window window(800, 700, "As8");
    window.SetState(FLAG_WINDOW_RESIZABLE);
    raylib::AudioDevice audio;
    
    Context ctx;

    auto e = ctx.CreateEntity();
    ctx.AddComponent<MapComponent>(e).player = 1;
    ctx.AddComponent<PlayerLocations>(e).all_posX.emplace_back(-1);
    ctx.GetComponent<PlayerLocations>(e).all_posY.emplace_back(-1);
    ctx.GetComponent<PlayerLocations>(e).all_posX.emplace_back(50);
    ctx.GetComponent<PlayerLocations>(e).all_posY.emplace_back(50);

    auto player = ctx.CreateEntity();
    ctx.AddComponent<PositionComponent>(player).posX = 50;
    ctx.AddComponent<Health>(player).hp = 10;
    ctx.GetComponent<Health>(player).armor = 2;
    ctx.GetComponent<PositionComponent>(player).posY = 50;
    ctx.AddComponent<Inventory>(player);
    ctx.GetComponent<Inventory>(player).open = false;


    SetupWorldSystem(ctx);

    int state = 0;

    while(!window.ShouldClose()) 
    {
        window.BeginDrawing(); 
        {
            if(state == 0)
            {
                window.ClearBackground(raylib::Color::Gray());
                DrawText("Hello!", 100, 100, 50, RED);
                DrawText("Press ENTER to begin", 100, 200, 30, BLUE);
                if(raylib::Keyboard::IsKeyPressed(KEY_ENTER))
                {
                    state++;
                }
            }
            else if(state == 1)
            {
                window.ClearBackground(raylib::Color::Black());
                InputHandlerSystem(ctx);
                DrawWorldSystem(ctx);
                PickupItemSystem(ctx);
            }    
            //window.DrawFPS();
        } window.EndDrawing();
    }
}