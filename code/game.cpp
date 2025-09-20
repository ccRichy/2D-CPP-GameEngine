#include "my_math.cpp"
#include "my_string.cpp"
#include "my_color.h"
#include "input.h"
#include "array_functions.h"

#include "game.h"

#include "render.cpp"
#include "collide.cpp"
#include "player.cpp"
#include "entity.cpp"



///LEVEL SHIT TODO: move this!!
void
level_save(const char* filename, Game_Pointers game_pointers)
{
    game_pointers.memory->DEBUG_platform_file_write_entire(filename, sizeof(Game_Entities), game_pointers.entity);
}
void
level_load(const char* filename, Game_Pointers game_pointers)
{
    DEBUG_File level = game_pointers.memory->DEBUG_platform_file_read_entire(filename);
    *game_pointers.entity = *(Game_Entities*)level.memory;
    game_pointers.data->level_current = filename;
}

enum class Ent_Type
{
    player,
    wall,
    enemy,
    null
};

globalvar Ent_Type global_editor_ent_to_spawn;
Ent_Type editor_pick_entity(Game_Input_Map input)
{
    if (input.num1) return Ent_Type::player;
    else if (input.num2) return Ent_Type::enemy;
    else if (input.num3) return Ent_Type::wall;
    else return Ent_Type::null;
}
void editor_entity_spawn(Ent_Type type, Vec2 pos, Game_Entities* entity)
{
    if (type == Ent_Type::player)     entity->player.Create(pos);
    else if (type == Ent_Type::wall)  entity->walls.Create(pos, {Tile(1), Tile(1)});
}




extern "C" GAME_INPUT_CHANGE_DEVICE(game_input_change_device)
{
    *input_map = {};
    input_map->game_input_device = input_device_current;
}

globalvar bool32 global_paused;
extern "C" GAME_UPDATE_AND_DRAW(game_update_and_draw)
{
    Game_Data* data = (Game_Data*)game_pointers->memory->permanent_storage;
    Game_Entities* entity = &data->entity;
    Game_Settings* settings = game_pointers->settings;
    
    
    //initialize
    if (!game_pointers->memory->is_initalized)
    {
        Assert( (&input.bottom_button - &input.buttons[0]) == (array_length(input.buttons) - 1) );

        data->state = Game_State::play;
        game_pointers->data = (Game_Data*)game_pointers->memory->permanent_storage;
        game_pointers->entity = &game_pointers->data->entity;
        game_pointers->memory->is_initalized = true;

        level_load(LEVEL_FIRST, *game_pointers);
    }

    Player* player = &entity->player;
    Walls* walls = &entity->walls;
    Enemys* enemys = &entity->enemys;
    Bullets* bullets = &entity->bullets;
    
    switch (data->state)
    {
        case Game_State::edit:{
            if (input.editor_toggle) data->state = Game_State::play;
        }break;

        case Game_State::play:{
            if (input.editor_toggle){
                data->state = Game_State::edit;
                level_load(LEVEL_FIRST, *game_pointers);
            }
            
            player->Update(*game_pointers, input);
            enemys->Update(*game_pointers);
            bullets->Update(*game_pointers);
        }break;
    }    


    //DEBUG
    if (input.right_click)
        entity_spawn(enemys, input.mouse_pos);

    if (input.middle_click){
        if (!input.shift.hold){
            level_save("test.lvl", *game_pointers);
        }else{
            level_load("test.lvl", *game_pointers);
        }
    }

    
    
    //update

    if (input.reset.press)  //NOTE: temp RESET
        player->pos = {50, 50};
        
    //draw
    walls->Draw(*game_pointers);
    enemys->Draw(*game_pointers);
    bullets->Draw(*game_pointers);
    player->Draw(*game_pointers);
    
    
    //draw mouse
    draw_rect(*game_pointers, input.mouse_pos, {2, 2}, MAGENTA);
    draw_rect(*game_pointers, input.mouse_pos, {1, 1}, LIME);
    
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
