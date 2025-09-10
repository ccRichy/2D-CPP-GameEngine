#include "my_math.cpp"
#include "my_color.h"
#include "input.h"
#include "array_functions.h"

#include "game.h"

#include "render.cpp"
#include "player.cpp"
#include "entity.cpp"


extern "C" GAME_INPUT_CHANGE_DEVICE(game_input_change_device)
{
    *input_map = {};
    input_map->game_input_device = input_device_current;
}


extern "C" GAME_UPDATE_AND_DRAW(game_update_and_draw)
{
    Game_State* game_state = (Game_State*)game_data->memory->permanent_storage;
    
    //initialize
    if (! game_data->memory->is_initalized)
    {
        Assert( (&input.bottom_button - &input.buttons[0]) == (array_length(input.buttons) - 1) );
        game_data->state = game_state;
        game_data->memory->is_initalized = true;
        
        game_state->player.Create({50, 50}, {Tile(1), Tile(1)}, red); //NOTE: parameters not used rn
        game_state->walls.Create({100, 50}, {Tile(1), Tile(8)}, white);
    }
    Game_Settings* settings = game_data->settings;

    Player* player = &game_state->player;
    Walls* walls = &game_state->walls;

    //update
    game_state->player.Update(input);

    //draw
    game_state->player.Draw(game_data);
    walls->Draw(game_data);
}




















//EXTRA
// internal void
// render_weird_gradient(Game_Render_Buffer* buffer, int blue_offset, int green_offset)
// {
//     uint8* row = (uint8*)buffer->memory; //type = separation
    
//     for (int Y = 0; Y < buffer->height; ++Y)
//     {
//         uint8* pixel = (uint8*)row; //reference to individal pixel 0xBBGRRXX //endian
//         for (int X = 0; X < buffer->width; ++X)
//         {
//             *pixel = (uint8)(X%2 + blue_offset);
//             pixel++;

//             *pixel = (uint8)(Y + green_offset);
//             pixel++;

//             *pixel = 0;
//             pixel++;

//             *pixel = 0;
//             pixel++;
//         }

//         row += buffer->pitch;
//     }
// }


// #if MY_WIN32
// #include <windows.h>
// BOOL WINAPI DllMain(
//   HINSTANCE hinstDLL,
//   DWORD     fdwReason,
//   LPVOID    lpvReserved
// )
// {
//     return TRUE;
// }
// #endif
