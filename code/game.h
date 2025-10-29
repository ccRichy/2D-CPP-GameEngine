/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */
#pragma once

#define LEVEL_FIRST "ledge"
#define UNSAVED_BACKUP_NAME "unsaved"
#define LEVEL_NAME_MAX_LEN 64
#define FPS_TARGET 60
#define SEC_PER_FRAME_TARGET  (1.0f/FPS_TARGET)

#define BASE_W 320
#define BASE_H 180
#define WINDOW_SCALE_DEFAULT 4
 
#define GSTATE_DEFAULT Game_State::Edit
#define GDRAW_MODE_DEFAULT Draw_Mode::World
#define GEDITOR_MODE_DEFAULT Editor_Mode::Entity

#define TILE_SIZE 8

#define TILEMAP_W 1024
#define TILEMAP_H 1024
#define Tile(value) (value * TILE_SIZE)




#include "game_platform.h"
#include "my_color.h"
#include "render.h"
#include "entity.h"
#include "text.h"
#include "player.h"



#define IF_DEBUG if (pointers->data->debug_mode_enabled)
#define DEBUG_MESSAGE_MAX 10                    //debug messages at top op screen
#define DEBUG_MESSAGE_LIFETIME_DEFAULT 160      //
#define DEBUG_MESSAGE_POS_DEFAULT {BASE_W/2, 0} //
#define DEBUG_MESSAGE_SCALE_DEFAULT {0.5f, 0.5f} //
struct Debug_Message
{
    char text[BUFF_LEN];
    int32 lifetime;
    int32 alpha;
};
struct Debug_Message_Queue
{
    bool32 is_active;
    Vec2f pos;
    Vec2f scale;
    Debug_Message current_message;
};



//////GAME//////
enum struct Game_State{ //TODO: pause
    Edit,
    Play
};
enum struct Draw_Mode{
    Null = -1,
    World,
    Gui
};
enum struct Editor_Mode
{
    Entity,
    Tile,
    Num
};


struct Game_Sprites
{
  //Characters
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
    
    //other
    Sprite sWall_anim;
    Sprite sBlob_small;
    
  //Meta
    Sprite sMouse_cursors;
    Sprite sDebug;
    
    
  //Items
    //rope
    Sprite sRope;
};



typedef int32 Tile;
struct Tilemap
{
    Vec2f pos;
    int32 grid_w;
    int32 grid_h;
    int32 tile_w;
    int32 tile_h;
    Tile grid[TILEMAP_H][TILEMAP_W];
};
struct Game_Entities
{
    Player player;

    union {
        Entity array[ENT_MAX_ALL() + 1];
        struct {
            Entity walls  [ ENT_MAX(Ent_Type::Wall) ];
            Entity enemys [ ENT_MAX(Ent_Type::Enemy) ];
            
            Entity bottom_entity; //REQUIRED: to validate union sizes match
        };
    };

    int32 nums[Ent_Type::Num];
    Entity* pointers[Ent_Type::Num];
};
struct Game_Data
{
    //settings
    Game_State state;
    Draw_Mode draw_mode;
    Editor_Mode editor_mode;

    //systems
    bool32 debug_mode_enabled;
    Debug_Message_Queue debug_msg;

    //resources
    Game_Sprites sprites;

    //world
    char level_current[LEVEL_NAME_MAX_LEN];
    Game_Entities entity;
    Tilemap tilemap;

    
    //misc //TODO: factor out/move
    Vec2f camera_pos;
    float32 camera_yoffset_extra;
    Vec2f camera_pos_offset_default;
    Vec2f camera_pos_offset;
    
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

struct Game_Settings //REQUIRED: do not 0 init this struct
{
    float32 window_scale = 4.0f; //doesnt exactly qualify as a "setting"
    float32 zoom_scale = 1.0f;   //should probably exist in the camera object
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
//NOTE: members populated in platform layer
struct Game_Pointers //just all the fuckin data
{
    Game_Memory*        memory;
    Game_Render_Buffer* render;
    Game_Sound_Buffer*  sound;
    Game_Input_Map*     input;
    Game_Performance*   performance;
    Game_Settings*      settings;
    Game_Data*          data;
    Game_Entities*      entity;
    Game_Sprites*       sprite;
    Player*             player;
    Typing_Buffer*      console;
};



//WARNING: global variables without default values will be 0'd when hot-reloading
// struct Game_Pointers;
//NOTE: initialized in game_init
globalvar Player* PLAYER;
globalvar Game_Data* GDATA; 
globalvar Game_Memory* GMEMORY;
globalvar Game_Input_Map* GIN;
globalvar Game_Sprites* GSPRITE;
globalvar Game_Entities* GENTITY;
globalvar Game_Settings* GSETTING;
globalvar Game_Pointers* pointers;
