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
    Game_Pointers pointers;
    
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

        //TODO: remove this vv later cuz this shit below is not stayin
        pointers = *game_pointers;

        //images
        data->sPlayer_idle = sprite_create(pointers, "sPlayer_idle.bmp", 2, 1);
        data->sPlayer_walk = sprite_create(pointers, "sPlayer_walk.bmp", 4, 5);
    #define BMP_LOAD(name) data->##name = DEBUG_load_bmp(pointers, #name ".bmp")
        BMP_LOAD(sTest);
        BMP_LOAD(sTest_wide);
        BMP_LOAD(sMan);
        BMP_LOAD(sMan_anim);
        BMP_LOAD(sFont_test);
        BMP_LOAD(sFont_ASCII_lilliput);

        //LEVEL TEMP
        player_create(pointers, {BASE_W/2, BASE_H/2});
        wall_create(pointers, {Tile(3), Tile(20)}, {Tile(18), Tile(1)});
        wall_create(pointers, {Tile(17), Tile(20)}, {Tile(18), Tile(1)});
        wall_create(pointers, {Tile(4), Tile(11)}, {Tile(18), Tile(1)});
        wall_create(pointers, {Tile(2), Tile(11)}, {Tile(1), Tile(10)});
        wall_create(pointers, {Tile(32), Tile(11)}, {Tile(1), Tile(10)});
        // enemy_create(pointers, {10, 10});
    }
    pointers = *game_pointers;

    draw_bmp(pointers, data->sTest, input.mouse_pos, {0.5, 0.5});
    // //TEXT TEST
    // const char* font_string = "HELP im stuck in a crappy game engine!!!11!";
    // BMP_File* font = data->sFont_ASCII_lilliput;
    // int32 frame_size = font->height;
    // int32 frame_num = font->width / frame_size;
    // for (int i = 0; i < string_length(font_string); ++i)
    // {
    //     int32 frame = font_string[i] - 33;
    //     draw_bmp_part(pointers, font, {10.f + ((frame_size-2) * i), 10}, {0.5, 0.5},
    //                   frame * frame_size, 0, //pos
    //                   frame_size, frame_size);//size
    // }
    
    //ZOOM TEST
    if (input.mouse_scroll > 0)
        game_pointers->settings->zoom_scale *= (float32)(sqrt(sqrt(2)));
    else if (input.mouse_scroll < 0)
        game_pointers->settings->zoom_scale /= (float32)(sqrt(sqrt(2)));

    if (input.mouse_back){
        game_pointers->settings->zoom_scale = 1;
    }

    
    //GAME_STATE
    if (data->state == Game_State::edit)
    {
        if (input.editor_toggle)
            data->state = Game_State::play;
    }
    else if (data->state == Game_State::play)
    {
        if (input.editor_toggle){
            data->state = Game_State::edit;
        }
            
        player_update(pointers, input);
        enemy_update(pointers);
    }


    //DEBUG
    if (input.reset){  //NOTE: temp RESET
        player->pos = {BASE_W/2, BASE_H/2};
        player->spd = {};
    }

    //draw
    wall_draw(pointers);
    enemy_draw(pointers);
    player_draw(player, pointers);
    
    //draw mouse
    draw_rect(pointers, input.mouse_pos, {2, 2}, MAGENTA);
    draw_rect(pointers, input.mouse_pos, {1, 1}, LIME);
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
