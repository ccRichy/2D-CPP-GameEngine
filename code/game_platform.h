#pragma once

#include "input.h"



#if MY_DEBUG
#define Assert(condition) if (!(condition)) {*(int*)0 = 0;}
#else
#define Assert(condition)
#endif


#define GAME_MEMORY_MB_PERMANENT 8
#define GAME_MEMORY_MB_TRANSIENT 16

#define SND_MAX_CONCURRENT    64
#define SND_SAMPLE_RATE       44100
#define SND_BITS_PER_SAMPLE   16
#define SND_CHANNELS          2
#define SND_BUFFER_SIZE_BYTES (((SND_SAMPLE_RATE) * (SND_BITS_PER_SAMPLE/8)))



/*
  TODO: PLATFORM -> GAME
*/

inline uint32 //TODO: where the fuck do we put this
safe_truncate_uint64(uint64 value)
{
    Assert(value <= 0xFFFFFFFF);
    return (uint32)value;
}


#if MY_INTERNAL
struct DEBUG_File
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
#endif







/*
  NOTE: GAME -> PLATFORM
*/
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


struct Game_Pointers;
#define GAME_UPDATE_AND_DRAW(name) void name(Game_Pointers* __game_pointers, Game_Input_Map input)
typedef GAME_UPDATE_AND_DRAW(Game_Update_And_Draw);
GAME_UPDATE_AND_DRAW(Game_Update_And_Draw_Stub) {}

//TODO: This should be on the platform layer. We can pass the device to the game if needed.
#define GAME_INPUT_CHANGE_DEVICE(name) void name(Game_Input_Device input_device_current, Game_Input_Map* input_map)
typedef GAME_INPUT_CHANGE_DEVICE(Game_Input_Change_Device);
GAME_INPUT_CHANGE_DEVICE(Game_Input_Change_Device_Stub) {}
