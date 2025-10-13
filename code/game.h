#pragma once

#include "game_platform.h"
#include "player.h"
#include "entity.h"
#include "render.h"
#include "text.h"




#define LEVEL_FIRST "test.lvl"
#define FPS_TARGET 60
#define SEC_PER_FRAME_TARGET  (1.0f/FPS_TARGET)

#define BASE_W 320
#define BASE_H 180
#define WINDOW_SCALE_DEFAULT 4

#define TILE_SIZE 8
 
#define GAME_STATE_DEFAULT Game_State::Edit
#define GAME_DRAW_MODE_DEFAULT Draw_Mode::World

#define Tile(value) (value * TILE_SIZE)


#define IF_DEBUG if (pointers->data->debug_mode_enabled)
#define DEBUG_MESSAGE_MAX 10                    //debug messages at top op screen
#define DEBUG_MESSAGE_LIFETIME_DEFAULT 160      //
#define DEBUG_MESSAGE_POS_DEFAULT {BASE_W/2, 0} //


struct Debug_Message
{
    char text[BUFF_LEN];
    int32 lifetime;
    int32 alpha;
};
struct Debug_Message_Queue
{
    bool32 is_active;
    Vec2f gui_pos;
    Debug_Message current_message;
};




//WARNING: global variables without default values will be 0'd upon recompilation
static Game_Pointers* pointers;
//convenience 
//
#define GAME_MEMORY pointers->memory      //
#define GAME_DATA pointers->data          //
#define GAME_ENTITY pointers->entity      //WARNING: deprecated
#define GAME_SETTINGS pointers->settings  //
#define GAME_SPRITE pointers->sprite      //




//////GAME//////
enum struct Game_State{
    Edit,
    Play
};

enum struct Draw_Mode{
    World,
    Gui
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

    int32 nums[Ent_Type::Num];
    const char* names[Ent_Type::Num];
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

  //Meta
    Sprite sMouse_cursors;
    
    
  //Items
    //rope
    Sprite sRope;
};
struct Game_Data
{
    //TODO: Game_Camera
    Game_State state;
    Draw_Mode draw_mode;
    Game_Entities entity;

    Debug_Message_Queue debug_msg;

    bool32 debug_mode_enabled;
    
    Vec2f camera_pos;
    float32 camera_yoffset_extra;
    Vec2f camera_pos_offset_default;
    Vec2f camera_pos_offset;

    Game_Sprites sprites;
    
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
    Game_Sprites*       sprite;
    Player*             player;
};
