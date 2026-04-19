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
#include <map>

raylib::AudioDevice audio;
raylib::Sound pickup("audio/itemPickup.mp3");
raylib::Sound luckyPickup("audio/luckyPickup.mp3");
raylib::Sound walk("audio/walk.mp3");

std::map<int, Color> itoC = 
{
    {0, RED},
    {1, GREEN},
    {2, DARKGREEN},
    {3, BLUE},
    {4, DARKBLUE},
    {5, GRAY},
    {6, DARKGRAY},
    {7, WHITE},
    {8, DARKBROWN},
    {9, PURPLE},
    {10, DARKPURPLE}
};

// enum Colors
// {
//     red = 0,
//     green,
//     blue,
//     white,
//     gray,
//     darkgreen,
//     darkblue,
//     darkbrown,
//     darkpurple,
//     darkgray
// };

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
    std::vector<int> all_colors;
};

struct Display
{
    const char* display;
    int count;
};

struct PlayerName
{
    std::string name;
};

struct PositionComponent
{
    int posX;
    int posY;
    bool moveable = true;
};

struct Inventory
{
    // Wood, Stone, Vine, Seeds
    int inventory[10];
    bool open;
    int item = 0;
};

/***************************************************************
**
**      BEGIN SYSTEMS
**
****************************************************************/

void inventoryDisplay(Context &ctx, entity e)
{
    auto &inv = ctx.GetComponent<Inventory>(e);

    for(int i = 0; i < 5; i++)
    {
        DrawText("|", 40, 36 * (14+i), 36, GRAY);
        DrawText("|", 760, 36 * (14 + i), 36, GRAY);
    }
    for(int i = 0; i < 20; i++)
    {
        DrawText("_", 18 + 36 * (i+1), 648, 36, GRAY);
    }
    DrawText(",", 100, 250, 360, BROWN);
    DrawText("o", 275, 440, 180, GRAY);
    DrawText("c", 450, 440, 180, DARKGREEN);
    DrawText("`", 625, 460, 360, GREEN);

    std::string qwood = "x"+ std::to_string(inv.inventory[0]);
    const char* qwooddisplay = qwood.c_str();
    std::string qstone = "x"+ std::to_string(inv.inventory[1]);
    const char* qstonedisplay = qstone.c_str();
    std::string qvine = "x"+ std::to_string(inv.inventory[2]);
    const char* qvinedisplay = qvine.c_str();
    std::string qseed = "x"+ std::to_string(inv.inventory[3]);
    const char* qseeddisplay = qseed.c_str();
    
    DrawText(qwooddisplay, 100, 600, 40, WHITE);
    DrawText(qstonedisplay, 275, 600, 40, WHITE);
    DrawText(qvinedisplay, 450, 600, 40, WHITE);
    DrawText(qseeddisplay, 625, 600, 40, WHITE);
}

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
        auto inv = ctx.GetComponent<Inventory>(1);
        auto map = ctx.GetComponent<MapComponent>(e);
        auto player_locations = ctx.GetComponent<PlayerLocations>(e);
        auto display = ctx.GetComponent<Display>(e);
        //auto player_names = ctx.GetComponent<PlayerNames>(e);
        //const char* name = player_names.all_names.at(1).c_str();
        //std::cout << player_names.all_names.at(1) << std::endl;
        int col, line;
        col = 1;
        if(display.count > 0)
        {
            DrawText(display.display, 50, 20, 20, YELLOW);
            display.count--;
        }
        
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
                    //DrawText(player_names.all_names.at(1).c_str(), 40 * col, 36 * (line), 20, itoC[player_locations.all_colors.at(map.player)]);
                    DrawText(player, 40 * col, 36 * (line+1), 36, itoC[player_locations.all_colors.at(map.player)]);
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
        if(inv.open)
        {
            inventoryDisplay(ctx, 1);
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
        if(raylib::Keyboard::IsKeyPressed(KEY_W) && position.moveable)
        {
            if(map.world[position.posX][position.posY - 1] != '^')
            {
                world.all_posY.at(e)--;
                position.posY--;
                walk.Play();
            }
        }
        else if(raylib::Keyboard::IsKeyPressed(KEY_S) && position.moveable)
        {
            if(map.world[position.posX][position.posY + 1] != '^')
            {
                world.all_posY.at(e)++;
                position.posY++;
                walk.Play();
            }
        }
        else if(raylib::Keyboard::IsKeyPressed(KEY_A) && position.moveable)
        {
            if(map.world[position.posX-1][position.posY] != '^')
            {
                world.all_posX.at(e)--;
                position.posX--;
                walk.Play();
            }
        }
        else if(raylib::Keyboard::IsKeyPressed(KEY_D) && position.moveable)
        {
            if(map.world[position.posX+1][position.posY] != '^')
            {
                world.all_posX.at(e)++;
                position.posX++;
                walk.Play();
            }
        }
        else if(raylib::Keyboard::IsKeyPressed(KEY_ENTER) && !inv.open)
        {
            inv.open = true;
            position.moveable = false;
        }
        else if(raylib::Keyboard::IsKeyPressed(KEY_ENTER) && inv.open)
        {
            inv.open = false;
            position.moveable = true;
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
        auto& dis = ctx.GetComponent<Display>(0);
        auto player_names = ctx.GetComponent<PlayerName>(e);
        //const char* name = player_names.name.c_str();
        std::string name = player_names.name;

        if(world.world[pos.posX][pos.posY] == ',')
        {
            inv.inventory[0]++;
            world.world[pos.posX][pos.posY] = 'x';
            std::string displayS = name + " picked up WOOD!";
            
            const char* display = displayS.c_str();std::cout << display << std::endl;
            dis.display = display;
            dis.count = 120;
            pickup.Play();
        }
        else if(world.world[pos.posX][pos.posY] == 'o')
        {
            inv.inventory[1]++;
            world.world[pos.posX][pos.posY] = 'x';
            std::string displayS = name + " picked up STONE!";
            const char* display = displayS.c_str();
            dis.display = display;
            dis.count = 120;
            pickup.Play();
        }
        else if(world.world[pos.posX][pos.posY] == 'c')
        {
            std::string displayS = name + " picked up VINE!";
            if(rand() % 10 == 0)
            {
                inv.inventory[3]++;
                displayS = displayS + " And found a SEED!";
                luckyPickup.Play();
            }
            else
            {
                pickup.Play();
            }
            inv.inventory[2]++;
            world.world[pos.posX][pos.posY] = 'x';
            
            const char* display = displayS.c_str();
            dis.display = display;
            dis.count = 120;
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
    window.SetTargetFPS(60);
    window.SetState(FLAG_WINDOW_RESIZABLE);
    
    
    Context ctx;

    auto e = ctx.CreateEntity();
    ctx.AddComponent<MapComponent>(e).player = 1;
    ctx.AddComponent<PlayerLocations>(e).all_posX.emplace_back(-1);
    ctx.GetComponent<PlayerLocations>(e).all_posY.emplace_back(-1);
    ctx.GetComponent<PlayerLocations>(e).all_colors.emplace_back(-1);
    ctx.GetComponent<PlayerLocations>(e).all_posX.emplace_back(50);
    ctx.GetComponent<PlayerLocations>(e).all_posY.emplace_back(50);
    ctx.AddComponent<Display>(e);
    

    auto player = ctx.CreateEntity();
    ctx.AddComponent<PositionComponent>(player).posX = 50;
    ctx.AddComponent<Health>(player).hp = 10;
    ctx.GetComponent<Health>(player).armor = 2;
    ctx.GetComponent<PositionComponent>(player).posY = 50;
    ctx.AddComponent<Inventory>(player);
    ctx.GetComponent<Inventory>(player).open = false;
    ctx.AddComponent<PlayerName>(player).name = "";

    SetupWorldSystem(ctx);

    int state = 0;
    int color = 0;
    std::string username;
    const char* display;
    // std::string qwood = "x"+ std::to_string(inv.inventory[0]);
    // const char* qwooddisplay = qwood.c_str();
    while(!window.ShouldClose()) 
    {
        window.BeginDrawing(); 
        {
            if(state == 0)
            {
                window.ClearBackground(raylib::Color::Gray());
                DrawText("World Game", 100, 100, 50, RED);
                DrawText("Select a Color (left/right arrow)", 100, 200, 40, BLUE);
                DrawText("COLOR", 100, 300, 40, itoC[color]);
                if(raylib::Keyboard::IsKeyPressed(KEY_LEFT))
                {
                    color--;
                    if(color < 0)
                    {
                        color = 10;
                    }
                }
                if(raylib::Keyboard::IsKeyPressed(KEY_RIGHT))
                {
                    color = (color + 1) % 11;
                }
                if(raylib::Keyboard::IsKeyPressed(KEY_ENTER))
                {
                    state++;
                    ctx.GetComponent<PlayerLocations>(e).all_colors.emplace_back(color);
                }
                DrawText("Press ENTER to continue", 100, 500, 30, BLUE);
            }
            else if(state == 1)
            {
                window.ClearBackground(raylib::Color::DarkGray());
                DrawText("Enter a Username", 100, 200, 40, BLUE);
                int character = raylib::Keyboard::GetCharPressed();
                if(character != 0)
                {
                    username += (char)character;
                }
                display = username.c_str();
                
                DrawText(display, 100, 300, 40, BLUE);
                if(raylib::Keyboard::IsKeyPressed(KEY_BACKSPACE))
                {
                    username.pop_back();
                }
                if(raylib::Keyboard::IsKeyPressed(KEY_ENTER))
                {
                    ctx.GetComponent<PlayerName>(player).name = username;
                    state++;
                }
            }
            else if(state == 2)
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