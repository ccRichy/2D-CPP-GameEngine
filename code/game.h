#pragma once
#include "my_types_keywords.h"
#include "my_vec.h"

#if MY_DEBUG
#define Assert(condition) if (!(condition)) {*(int*)0 = 0;}
#else
#define Assert(condition)
#endif

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define FPS                   60
#define SEC_PER_FRAME         (1.0f/FPS)
#define SND_CHANNELS          2
#define SND_SAMPLE_RATE       44100
#define SND_BITS_PER_SAMPLE   16
#define SND_BUFFER_SIZE_BYTES (((SND_SAMPLE_RATE) * (SND_BITS_PER_SAMPLE/8)))
#define SND_MAX_CONCURRENT 64


inline uint32
safe_truncate_uint64(uint64 value)
{
    Assert(value <= 0xFFFFFFFF);
    return (uint32)value;
}



/*
  TODO: PLATFORM -> GAME
*/
#if MY_INTERNAL
struct DEBUG_File
{
    void* memory;
    uint32 size;
};

#define DEBUG_PLATORM_FILE_FREE_MEMORY(name) void name(void* memory)
typedef DEBUG_PLATORM_FILE_FREE_MEMORY(DEBUG_Platform_File_Free_Memory);

#define DEBUG_PLATORM_FILE_READ_ENTIRE(name) DEBUG_File name(const char* name)
typedef DEBUG_PLATORM_FILE_READ_ENTIRE(DEBUG_Platform_File_Read_Entire);

#define DEBUG_PLATORM_FILE_WRITE_ENTIRE(name) bool32 name(const char* name, uint32 memory_size, void* memory)
typedef DEBUG_PLATORM_FILE_WRITE_ENTIRE(DEBUG_Platform_File_Write_Entire);
#endif

/*
  NOTE: GAME -> PLATFORM
*/
struct Game_State
{
    int blue_offset;
    int green_offset;
    int sin_hz;
};
struct Game_Memory
{
    bool32 is_initalized;
    
    uint64 permanent_storage_space;
    void*  permanent_storage; //REQUIRED: clear to 0 upon allocation

    uint64 transient_storage_space;
    void*  transient_storage; //REQUIRED: clear to 0 upon allocation

#if MY_INTERNAL
    DEBUG_Platform_File_Read_Entire* DEBUG_platform_file_read_entire;
    DEBUG_Platform_File_Free_Memory* DEBUG_platform_file_free_memory;
    DEBUG_Platform_File_Write_Entire* DEBUG_platform_file_write_entire;
#endif
};

enum struct Game_Input_Device
{
    keyboard_mouse,
    controller
};

struct Game_Offscreen_Buffer
{
    void* memory;
    int pitch;
    int width;
    int height;
};
struct Game_Sound_Buffer
{
    int channels;
    int sample_rate;
    int16* memory;
};

struct Game_Input_Button
{
    bool32 press;
    bool32 hold;
    bool32 release;
};

struct Game_Input_Map
{
    Vec2 l_axes;
    Vec2 r_axes;
    
    union {
        Game_Input_Button buttons[5];
        struct {
            Game_Input_Button up;
            Game_Input_Button down;
            Game_Input_Button left;
            Game_Input_Button right;

            //REQUIRED: bottom of this struct for assert check
            Game_Input_Button bottom_button;
        };
    };
};


#define GAME_UPDATE_AND_DRAW(name) void name(Game_Memory* game_memory, Game_Input_Map input_map, Game_Offscreen_Buffer* render_buffer)
typedef GAME_UPDATE_AND_DRAW(Game_Update_And_Draw);
GAME_UPDATE_AND_DRAW(Game_Update_And_Draw_Stub) {}

#define GAME_INPUT_CHANGE_DEVICE(name) void name(Game_Input_Device input_device, Game_Input_Map* input_map)
typedef GAME_INPUT_CHANGE_DEVICE(Game_Input_Change_Device);
GAME_INPUT_CHANGE_DEVICE(Game_Input_Change_Device_Stub) {}



globalvar Game_Input_Device game_input_device = Game_Input_Device::keyboard_mouse;


//internal void sound_output(Game_Sound_Buffer* sound_buffer, int hz);
//internal void render_weird_gradient(Game_Offscreen_Buffer* buffer, int blue_offset, int green_offset);

//void game_input_change_device(Game_Input_Device input_device, Game_Input_Map* input_map);
///void game_update_and_draw(Game_Memory* game_memory, Game_Input_Map input_map, Game_Offscreen_Buffer* render_buffer);

