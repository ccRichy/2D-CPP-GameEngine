#include "my_math.cpp"
#include "my_color.h"
#include "input.h"
#include "array_functions.h"

#include "game.h"

#include "render.cpp"
#include "collide.cpp"
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
    if (!game_data->memory->is_initalized)
    {
        Assert( (&input.bottom_button - &input.buttons[0]) == (array_length(input.buttons) - 1) );
        game_data->state = (Game_State*)game_data->memory->permanent_storage;
        game_data->memory->is_initalized = true;
        
        game_state->player.Create(
            {50, 50},
            {Tile(1), Tile(2)},
            GREEN);
        game_state->walls.Create(
            {Tile(4), Tile(16)},
            {Tile(32), Tile(1)},
            WHITE);
        game_state->enemys.Create(
            {Tile(4), Tile(14)},
            {Tile(1), Tile(2)},
            RED);
    }
    Game_Settings* settings = game_data->settings;
    Game_Input_Map INP = input;

    Player* player = &game_state->player;
    Walls* walls = &game_state->walls;
    Enemys* enemys = &game_state->enemys;
    Bullets* bullets = &game_state->bullets;
    
    //update
    if (INP.reset.press)  //NOTE: temp RESET
        player->pos = {50, 50}; 
    player->Update(*game_data, input);
    enemys->Update(*game_data);
    bullets->Update(*game_data);

    //draw
    walls->Draw(*game_data);
    enemys->Draw(*game_data);
    bullets->Draw(*game_data);
    player->Draw(*game_data);

    //clean
    enemys->Cleanup_End_Frame();
    bullets->Cleanup_End_Frame();
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
