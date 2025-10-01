#include "my_types_constants.h"
#include "my_math.cpp"
#include "my_string.cpp"
#include "my_color.h"
#include "array_functions.h"

#include "game.h"

#include "render.cpp"
#include "collide.cpp"
#include "player.cpp"
#include "entity.cpp"





//file loading NOTE: move out of here?
BMP_File*
DEBUG_load_bmp(Game_Pointers game_pointers, const char* filename)
{
    return (BMP_File*)game_pointers.memory->DEBUG_platform_file_read_entire(filename).memory;
};

Sprite
sprite_create(Game_Pointers game_pointers, const char* bmp_filename, int32 frame_num, float32 fps)
{
    Sprite sprite = {};   
    sprite.bmp = DEBUG_load_bmp(game_pointers, bmp_filename);
    sprite.origin = {};
    sprite.fps = fps;
    sprite.frame_num = frame_num;
    sprite.is_animation = (frame_num > 0);
    return sprite;
}



///editor stuff
void
DEBUG_level_save(const char* filename, Game_Pointers game_pointers)
{
    if (true) return;
    game_pointers.memory->DEBUG_platform_file_write_entire(filename, sizeof(Game_Entities), game_pointers.entity);
}
void
DEBUG_level_load(const char* filename, Game_Pointers game_pointers)
{
    if (true) return;
    DEBUG_File level = game_pointers.memory->DEBUG_platform_file_read_entire(filename);
    //TODO: error handle
    *game_pointers.entity = *(Game_Entities*)level.memory;
    game_pointers.data->level_current = filename;
}

globalvar Ent_Type global_editor_ent_to_spawn; //NOTE: factor out
Ent_Type editor_pick_entity(Game_Input_Map input)
{
    auto return_val = global_editor_ent_to_spawn;
    if (input.num1)      return_val = Ent_Type::player;
    else if (input.num2) return_val = Ent_Type::enemy;
    else if (input.num3) return_val = Ent_Type::wall;
    return return_val;
}


//
extern "C" GAME_INPUT_CHANGE_DEVICE(game_input_change_device)
{
    *input_map = {};
    input_map->game_input_device = input_device_current;
}

extern "C" GAME_UPDATE_AND_DRAW(game_update_and_draw)
{
    Game_Data* data = (Game_Data*)game_pointers->memory->permanent_storage;
    Game_Entities* entity = &data->entity;
    Game_Settings* settings = game_pointers->settings;

    Player* player = &entity->player;
    
    //initialize
    if (!game_pointers->memory->is_initalized)
    {
        Assert( (&input.bottom_button - &input.buttons[0]) == (array_length(input.buttons) - 1) );
        Assert( sizeof(Game_Data) <= game_pointers->memory->permanent_storage_space );

        data->state = Game_State::play;
        game_pointers->data = (Game_Data*)game_pointers->memory->permanent_storage;
        game_pointers->entity = &game_pointers->data->entity;
        game_pointers->memory->is_initalized = true;

#define BMP_LOAD(name) data->##name = DEBUG_load_bmp(*game_pointers, #name ".bmp")
        data->sPlayer_idle = sprite_create(*game_pointers, "sPlayer_idle.bmp", 2, 1);
        data->sPlayer_walk = sprite_create(*game_pointers, "sPlayer_walk.bmp", 4, 5);
        
        BMP_LOAD(sTest);
        BMP_LOAD(sTest_wide);
        BMP_LOAD(sMan);
        BMP_LOAD(sMan_anim);
                
        player_create(*game_pointers, {BASE_W/2, BASE_H/2});
        wall_create(*game_pointers, {Tile(3), Tile(20)}, {Tile(18), Tile(1)});
        wall_create(*game_pointers, {Tile(17), Tile(20)}, {Tile(18), Tile(1)});
        wall_create(*game_pointers, {Tile(4), Tile(11)}, {Tile(18), Tile(1)});
        wall_create(*game_pointers, {Tile(2), Tile(11)}, {Tile(1), Tile(10)});
        wall_create(*game_pointers, {Tile(32), Tile(11)}, {Tile(1), Tile(10)});
        enemy_create(*game_pointers, {10, 10});
    }

    //GAME_STATE
    if (data->state == Game_State::edit)
    {
        if (input.editor_toggle)
            data->state = Game_State::play;

        if (input.ctrl.hold)
        {
            if (input.editor_save_level)
                DEBUG_level_save("test.lvl", *game_pointers);
            if (input.editor_load_level)
                DEBUG_level_load("test.lvl", *game_pointers);
        }

            
        global_editor_ent_to_spawn = editor_pick_entity(input);
        if (input.left_click){
            //TODO: spawn entity
        }
            
    }
    else if (data->state == Game_State::play)
    {
        if (input.editor_toggle){
            data->state = Game_State::edit;
            DEBUG_level_load(LEVEL_FIRST, *game_pointers);
        }
            
        player_update(*game_pointers, input);
        enemy_update(*game_pointers);
    }


    //DEBUG
    if (input.reset){  //NOTE: temp RESET
        player->pos = {BASE_W/2, BASE_H/2};
        player->spd = {};
    }
        // DEBUG_level_load(data->level_current, *game_pointers);

    
    //draw
    wall_draw(*game_pointers);
    enemy_draw(*game_pointers);
    player_draw(player, *game_pointers);

    persist float32 draw_y = 0;
    persist float32 draw_x = 0;
    if (input.jump) draw_x -= 0.05f;
    
    //draw mouse
    draw_rect(*game_pointers, input.mouse_pos, {2, 2}, MAGENTA);
    draw_rect(*game_pointers, input.mouse_pos, {1, 1}, LIME);
    
    //clean
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
