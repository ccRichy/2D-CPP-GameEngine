/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */
#pragma once
#if MY_DEBUG
#define Assert(condition) if (!(condition)) {*(int*)0 = 0;}
#else
#define Assert(condition)
#endif




//TODO: move to a debug file?
#define GAME_MEMORY_MB_PERMANENT  8
#define GAME_MEMORY_MB_TRANSIENT  128
#define SAVE_FILE_BUFFER_MB       10

#define SND_CHANNELS          2
#define SND_SAMPLE_RATE       44100
#define SND_MAX_CONCURRENT    64
#define SND_BITS_PER_SAMPLE   16
#define SND_BUFFER_SIZE_BYTES (((SND_SAMPLE_RATE) * (SND_BITS_PER_SAMPLE/8)))


#define LEVEL_FIRST "ledge"
#define UNSAVED_BACKUP_NAME "unsaved"
#define LEVEL_NAME_MAX_LEN 64
#define FPS_TARGET 60
#define SEC_PER_FRAME_TARGET  (1.0f/FPS_TARGET)

#define BASE_W 320
#define BASE_H 180
#define BASE_CENTER_V2F {BASE_W/2, BASE_H/2}
#define WINDOW_SCALE_DEFAULT 4
 
#define GSTATE_DEFAULT Game_State::Edit
#define GDRAW_MODE_DEFAULT Draw_Mode::World
#define GEDITOR_MODE_DEFAULT Editor_Mode::Entity
#define BUFF_LEN 256

#define IF_DEBUG if (pointers->data->debug_mode_enabled)
#define DEBUG_MESSAGE_MAX 10                    //debug messages at top op screen
#define DEBUG_MESSAGE_LIFETIME_DEFAULT 160      //
#define DEBUG_MESSAGE_POS_DEFAULT {BASE_W/2, 0} //
#define DEBUG_MESSAGE_SCALE_DEFAULT {0.5f, 0.5f} //







#include "my_types_constants.h"

//1. use an existing piece of allocated memory
//2. allocate arbitrarily out of it
//3. auto bounds check
struct MemAlloc
{
    void* root_pointer; //NOTE: default
    int32 size_bytes;
    int32 bytes_used;

    void* init(void* memory_location, i32 size_of_chunk){
        size_bytes = size_of_chunk;
        root_pointer = memory_location;
        return root_pointer;
    }
    
    void* end(){
        return (u8*)root_pointer + size_bytes;
    }
    
    void* alloc(i32 size){
        u8* new_ptr = (u8*)root_pointer + size;
        //bounds check
        if (new_ptr > end())
        bytes_used += size;
        return new_ptr;
    }

};


#include "my_array.cpp"
#include "my_string.cpp"
#include "my_math.cpp"
#include "my_color.h"
#include "input.cpp"
#include "render.h"
#include "text.h"
#include "tilemap.h"
#include "entity.h"
#include "player.h"



//PLATFORM
#if MY_INTERNAL
struct DEBUG_File
{
    void* memory;
    uint32 size;
};

struct Save_File
{
    void* memory;
    uint32 size;
};

#define DEBUG_PLATORM_FILE_FREE_MEMORY(name) void name(void* memory)
typedef DEBUG_PLATORM_FILE_FREE_MEMORY(DEBUG_Platform_File_Free_Memory);

#define DEBUG_PLATORM_FILE_READ_ENTIRE(name) DEBUG_File name(const char* filename)
typedef DEBUG_PLATORM_FILE_READ_ENTIRE(DEBUG_Platform_File_Read_Entire);

#define DEBUG_PLATORM_FILE_WRITE_ENTIRE(name) bool32 name(const char* filename, uint32 memory_size, void* memory)
typedef DEBUG_PLATORM_FILE_WRITE_ENTIRE(DEBUG_Platform_File_Write_Entire);

//new ones
#define DEBUG_PLATORM_FILE_OPEN(name) bool32 name(const char* filename)
typedef DEBUG_PLATORM_FILE_OPEN(DEBUG_Platform_File_Open);
#define DEBUG_PLATORM_FILE_CLOSE(name) bool32 name(const char* filename)
typedef DEBUG_PLATORM_FILE_CLOSE(DEBUG_Platform_File_Close);
#define DEBUG_PLATORM_FILE_APPEND(name) bool32 name(const char* filename)
typedef DEBUG_PLATORM_FILE_APPEND(DEBUG_Platform_File_Append);

#endif




//NOTE: GAME -> PLATFORM
struct Game_Memory
{
    bool32 is_initalized;
    
    uint64 permanent_storage_space;
    void*  permanent_storage; //REQUIRED: clear to 0 upon allocation

    uint64 transient_storage_space;
    void*  transient_storage; //REQUIRED: clear to 0 upon allocation

#if MY_INTERNAL
    DEBUG_Platform_File_Free_Memory*  DEBUG_platform_file_free_memory;
    DEBUG_Platform_File_Read_Entire*  DEBUG_platform_file_read_entire;
    DEBUG_Platform_File_Write_Entire* DEBUG_platform_file_write_entire;

    DEBUG_Platform_File_Open* DEBUG_platform_file_open;
    DEBUG_Platform_File_Close* DEBUG_platform_file_close;
    DEBUG_Platform_File_Append* DEBUG_platform_append_close;
    // DEBUG_Platform_File_Open* DEBUG_platform_file_open;
    // DEBUG_Platform_File_Close* DEBUG_platform_file_close;
#endif
};

struct Game_Render_Buffer
{
    void* memory;
    int pitch;
    int width;
    int height;
};

struct Game_Sound_Buffer
{
    int32  channels;
    int32  sample_rate;
    int16* memory;
};






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
    Play,
    Pause,
};
enum struct Draw_Mode{ //REQUIRED: draw calls are responsible for setting draw_mode
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
    Sprite sPlayer_ledge_grab;
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
    Sprite sPlayer_hurt;
    Sprite sPlayer_bounce;
    Sprite sPlayer_roll;
    
//Entities
    //movers
    Sprite sWall_anim;
    Sprite sBlob_small;
    //idlers
    Sprite sGoal;
    Sprite sSpike;
    //item
    Sprite sItem_rope;
    Sprite sItem_orb;

//Backgrounds
    Sprite sBG_test;
    
//Meta
    Sprite sMouse_cursors;
    Sprite sDebug;    
    
  //Items
    Sprite sRope;
};

struct Game_Entities
{
    Player player;

    union {
        Entity array[ENT_MAX_ALL()];
    };

    int32 nums[(i32)Ent_Type::Num];
    Entity* pointers[(i32)Ent_Type::Num];
};


struct Game_Data
{
    //settings
    MemAlloc memory_map;
    
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
    bool32  subpixel_rendering_enabled = false;
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
globalvar Game_Input_Map* GINPUT;
globalvar Game_Sprites* GSPRITE;
globalvar Game_Entities* GENTITY;
globalvar Game_Settings* GSETTING;
globalvar Game_Pointers* pointers;


#define GAME_UPDATE_AND_DRAW(name) void name(Game_Pointers* __game_pointers, Game_Input_Map* input)
typedef GAME_UPDATE_AND_DRAW(Game_Update_And_Draw);
GAME_UPDATE_AND_DRAW(Game_Update_And_Draw_Stub) {}

//TODO: This should be on the platform layer. We can pass the device to the game if needed.
#define GAME_INPUT_CHANGE_DEVICE(name) void name(Game_Input_Device input_device_current, Game_Input_Map* input_map)
typedef GAME_INPUT_CHANGE_DEVICE(Game_Input_Change_Device);
GAME_INPUT_CHANGE_DEVICE(Game_Input_Change_Device_Stub) {}
