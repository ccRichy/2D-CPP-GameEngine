#pragma once

#include "game_platform.h"
#include "player.h"
#include "entity.h"


#define LEVEL_FIRST "test.lvl"
#define FPS_TARGET 60
#define SEC_PER_FRAME_TARGET  (1.0f/FPS_TARGET)

#define BASE_W 320
#define BASE_H 180
#define WINDOW_SCALE_DEFAULT 4

#define TILE_SIZE 8
#define Tile(value) (value * TILE_SIZE)


enum struct Game_State
{
    edit,
    play
};


struct Game_Settings //REQUIRED: give members default value
{
    int8 window_scale = 4;
};
struct Game_Entities
{
    Player  player;
    Walls   walls;
    Enemys  enemys;
    Bullets bullets;
};
struct Game_Data
{
    BMP_File* bmp_file;
    
    const char* level_current;
    Game_State state;
    Game_Entities entity;
};
struct Game_Pointers //just all the fuckin data
{
    Game_Render_Buffer* render;
    Game_Memory*        memory;
    Game_Sound_Buffer*  sound;
    Game_Settings*      settings;
    
    Game_Data* data; //populated in game init
    Game_Entities* entity; //populated in game init
};
