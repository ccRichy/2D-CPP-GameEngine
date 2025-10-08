#pragma once

#include "game_platform.h"
#include "player.h"
#include "entity.h"
#include "render.h"

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


//NOTE: global variables without default values will be 0'd upon recompilation

//convenience //TODO: DELETE THESE!!!
#define GAME_POINTERS pointers->data
#define GAME_MEMORY pointers->data
#define GAME_DATA pointers->data
#define GAME_ENTITY pointers->entity
#define GAME_SETTINGS pointers->settings

#define IF_DEBUG if (GAME_DATA->debug_mode_enabled)

static Game_Pointers* pointers;


struct Sprite
{
    BMP_Data bmp;
	Vec2 origin;
    float32 fps;
    uint32 frame_num;
    bool32 is_animation;
};

#define FONT_GLYPH_SIZE 8
#define FONT_LENGTH 94
#define FONT_ASCII_CHARACTER_START_OFFSET 32
struct Glyph
{
    uint32 pixels[FONT_GLYPH_SIZE][FONT_GLYPH_SIZE];
};
struct Font
{
    BMP_Data* image;
    int32 glyph_width;
    int32 glyph_height;
    Glyph glyphs[FONT_LENGTH]; 
};



//////GAME//////
enum struct Game_State{
    edit,
    play
};

enum struct Draw_Mode{
    world,
    gui
};

struct Game_Entities
{
    Player player;

    union {
        Entity array[ENT_MAX + 1];
        struct {
            Entity walls[WALL_MAX];
            Entity enemys[ENEMY_MAX];
            
            Entity bottom_entity; //REQUIRED: to validate union sizes match
        };
    };

    int32 nums[Ent_Type::num];
    const char* names[Ent_Type::num];
};
struct Game_Data
{
    //TODO: Game_Camera
    Game_State state;
    Draw_Mode draw_mode;
    Game_Entities entity;

    bool32 debug_mode_enabled;
    
    Vec2 camera_pos;
    float32 camera_yoffset_extra;
    Vec2 camera_pos_offset_default;
    Vec2 camera_pos_offset;
    
  //Sprites //TODO: Move out into their own struct, and append Game_Pointers
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
    BMP_Data sTest;
    BMP_Data sTest_wide;
    BMP_Data sMan;
    BMP_Data sMan_anim;
    BMP_Data sFont_test;
    BMP_Data sFont_ASCII_lilliput;
    BMP_Data sFont_ASCII_lilliput_vert;

  //Fonts
    Font font_test;
};

struct Game_Settings //REQUIRED: give members default value
{
    //TODO: move into game_data //it may not qualifty as game data once the progam is more complex,
                                //but its DEFINITELY not a SETTING
    float32 window_scale = 4.0f;
    float32 zoom_scale = 1.0f;
};
struct Game_Performance //TODO: averages
{
    float32 fps;
    
    float64 ms_frame;
    float64 megacycles_frame;
    
    float64 ms_update;
    float64 megacycles_update;

    float64 ms_render;
    float64 megacycles_render;
};

//NOTE: initialized in platform layer
struct Game_Pointers //just all the fuckin data
{
    Game_Memory*        memory;
    Game_Render_Buffer* render;
    Game_Sound_Buffer*  sound;
    Game_Settings*      settings;
    Game_Input_Map*     input;
    Game_Performance*   performance;
    Game_Data*          data;
    Game_Entities*      entity;
    Player*             player;
};
