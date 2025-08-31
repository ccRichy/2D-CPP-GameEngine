#include "game.h"
#include "my_types_keywords.h"
#include "my_math.h"
#include "array_functions.h"





internal void
render_weird_gradient(Game_Offscreen_Buffer* buffer, int blue_offset, int green_offset)
{
    uint8* row = (uint8*)buffer->memory; //type = separation
    
    for (int Y = 0; Y < buffer->height; ++Y)
    {
        uint8* pixel = (uint8*)row; //reference to individal pixel 0xBBGRRXX //endian
        for (int X = 0; X < buffer->width; ++X)
        {
            *pixel = (uint8)(X + blue_offset);
            pixel++;

            *pixel = (uint8)(Y + green_offset);
            pixel++;

            *pixel = 0;
            pixel++;

            *pixel = 0;
            pixel++;
        }

        row += buffer->pitch;
    }
}



extern "C" GAME_INPUT_CHANGE_DEVICE(game_input_change_device)
{
    game_input_device = input_device;
    *input_map = {};
}

extern "C" GAME_UPDATE_AND_DRAW(game_update_and_draw)
{
    Game_State* game_state = (Game_State*)game_memory->permanent_storage;
    if (!game_memory->is_initalized)
    {
#if 0
        //file test
        DEBUG_File file = game_memory->DEBUG_platform_file_read_entire(__FILE__);
        if (file.memory)
        {
            game_memory->DEBUG_platform_file_write_entire("test.txt", file.size, file.memory);
            game_memory->DEBUG_platform_file_free_memory(file.memory);
        }
#endif

        //error check
        Assert(
            (&input_map.bottom_button - &input_map.buttons[0]) ==
            (array_length(input_map.buttons) - 1)
        );
        
        //init data
        game_state->sin_hz = 256;
        game_memory->is_initalized = true;
    }

    //game
    int spd = 10;
    game_state->green_offset += (input_map.up.hold - input_map.down.hold) * spd;
    game_state->blue_offset += (input_map.left.hold - input_map.right.hold) * spd;
    game_state->green_offset -= (int)(input_map.l_axes.y * spd);
    game_state->blue_offset += (int)(input_map.l_axes.x * spd);
    
    render_weird_gradient(render_buffer, game_state->blue_offset, game_state->green_offset);
}

#if MY_WIN32
#include <windows.h>
BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,
  DWORD     fdwReason,
  LPVOID    lpvReserved
)
{
    return TRUE;
}
#endif
