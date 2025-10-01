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

#define G_DATA game_pointers.data


enum struct Game_State
{
    edit,
    play
};


#pragma pack(push, 1)
struct BMP_File
{
    uint16 type;
    uint32 size;
    uint16 reserved1;
    uint16 reserved2;
    uint32 offset;
    uint32 info_size;
    int32  width;
    int32  height;
    uint16 planes;
    uint16 bits_per_pixel;
};
#pragma pack(pop)
struct Sprite
{
    BMP_File* bmp;
	Vec2 origin;
    float32 fps;
    int32 frame_num;
    bool32 is_animation;
};



struct Game_Settings //REQUIRED: give members default value
{
    int8 window_scale = 4;
};
struct Game_Entities
{
    Player player;
    Entity walls[WALL_MAX];
    int32  wall_num;
    Entity enemys[ENEMY_MAX];
    int32  enemy_num;
};
struct Game_Data
{
  //Sprites
    //player
    Sprite sPlayer_idle;
    Sprite sPlayer_walk;

    //misc
    BMP_File* sTest;
    BMP_File* sTest_wide;
    BMP_File* sMan;
    BMP_File* sMan_anim;
    
    
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
