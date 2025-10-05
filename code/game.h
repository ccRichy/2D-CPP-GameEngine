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

#define GAME_STATE_DEFAULT Game_State::play;
#define GAME_DRAW_MODE_DEFAULT Draw_Mode::world;

//convenience
#define GAME_POINTERS game_pointers->data
#define GAME_MEMORY game_pointers->data
#define GAME_DATA game_pointers->data
#define GAME_ENTITY game_pointers->entity
#define GAME_SETTINGS game_pointers->settings

enum struct Game_State{
    edit,
    play
};

enum struct Draw_Mode{
    world,
    gui
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
    uint32 frame_num;
    bool32 is_animation;
};



struct Game_Settings //REQUIRED: give members default value
{
    //TODO: move into game_data //it may not qualifty as game data once the progam is more complex,
                                //but its DEFINITELY not a SETTING
    float32 window_scale = 4.0f;
    float32 zoom_scale = 1.0f;
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
    Sprite sPlayer_air;
    Sprite sPlayer_air_reach;
    Sprite sPlayer_idle;
    Sprite sPlayer_ledge;
    Sprite sPlayer_ledge_reach;
    Sprite sPlayer_rope_climb;
    Sprite sPlayer_rope_slide;
    Sprite sPlayer_splat_slow;
    Sprite sPlayer_splat_swift;
    Sprite sPlayer_turn;
    Sprite sPlayer_walk;
    Sprite sPlayer_walk_reach;
    Sprite sPlayer_wire_idle;
    Sprite sPlayer_wire_walk;
    //rope
    Sprite sRope;

    //misc
    BMP_File* sTest;
    BMP_File* sTest_wide;
    BMP_File* sMan;
    BMP_File* sMan_anim;
    BMP_File* sFont_test;
    BMP_File* sFont_ASCII_lilliput;

    // const char* level_current;
    Game_State state;
    Draw_Mode draw_mode;
    Game_Entities entity;
    
    Vec2 camera_pos;
    float32 camera_yoffset_extra = 4;
    Vec2 camera_pos_offset_default;
    Vec2 camera_pos_offset;
    bool32 camera_panning;
};
struct Game_Pointers //just all the fuckin data
{
    //initialized in platform layer
    Game_Memory*        memory;
    Game_Render_Buffer* render;
    Game_Sound_Buffer*  sound;
    Game_Settings*      settings;
    Game_Input_Map*     input;

    //initialized in game layer (here)
    Game_Data* data; //populated in game init
    Game_Entities* entity;
    Player* player;
};
