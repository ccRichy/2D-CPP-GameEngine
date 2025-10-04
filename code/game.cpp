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




//text drawing
void draw_text(Game_Pointers* game_pointers, const char* text, Vec2 pos, Vec2 scale = {1, 1}, int32 hori_spacing = 5)
{
    //TEXT TEST
    BMP_File* font_image = game_pointers->data->sFont_ASCII_lilliput;
    int32 frame_size = font_image->height;
    for (int i = 0; i < string_length(text); ++i)
    {
        int32 frame = text[i] - 32;
        draw_bmp_part(game_pointers, font_image, {pos.x + ((hori_spacing * scale.x) * i), pos.y}, {scale.x, scale.y},
                      frame * frame_size, 0, //pos
                      frame_size, frame_size);//size
    }
    
}



//file loading NOTE: move out of here?
BMP_File*
DEBUG_load_bmp(Game_Pointers* game_pointers, const char* filename)
{
    return (BMP_File*)game_pointers->memory->DEBUG_platform_file_read_entire(filename).memory;
};
Sprite
sprite_create(Game_Pointers* game_pointers, const char* bmp_filename, uint32 frame_num, float32 fps)
{
    Sprite sprite = {};   
    sprite.bmp = DEBUG_load_bmp(game_pointers, bmp_filename);
    sprite.origin = {};
    sprite.fps = fps;
    sprite.frame_num = frame_num;
    sprite.is_animation = (frame_num > 0 && fps != 0);
    return sprite;
}



//
extern "C" GAME_INPUT_CHANGE_DEVICE(game_input_change_device)
{
    *input_map = {};
    input_map->game_input_device = input_device_current;
}

#include "stdio.h"
extern "C" GAME_UPDATE_AND_DRAW(game_update_and_draw)
{
    Game_Data* data = (Game_Data*)game_pointers->memory->permanent_storage;
    Game_Entities* entity = &data->entity;
    Game_Settings* settings = game_pointers->settings;
    Game_Pointers* pointers = game_pointers;
    
    Player* player = &entity->player;
    
    //initialize
    if (!game_pointers->memory->is_initalized)
    {
        Assert( (&input.bottom_button - &input.buttons[0]) == (array_length(input.buttons) - 1) );
        Assert( sizeof(Game_Data) <= game_pointers->memory->permanent_storage_space );

        data->state = Game_State::play;
        data->draw_mode = GAME_DRAW_MODE_DEFAULT;
        game_pointers->entity = &data->entity;
        game_pointers->player = &data->entity.player;
        game_pointers->data = (Game_Data*)game_pointers->memory->permanent_storage;
        game_pointers->memory->is_initalized = true;
        //TODO: remove this vv later cuz this shit below is not stayin

        //images
        data->sPlayer_air         = sprite_create(pointers, "sPlayer_air2.bmp", 7, 15);
        data->sPlayer_air_reach   = sprite_create(pointers, "sPlayer_air_reach.bmp", 2, 0);  
        data->sPlayer_idle        = sprite_create(pointers, "sPlayer_idle.bmp", 2, 0);               
        data->sPlayer_ledge       = sprite_create(pointers, "sPlayer_ledge.bmp", 4, 10);      
        data->sPlayer_ledge_reach = sprite_create(pointers, "sPlayer_ledge_reach.bmp", 3, 0);
        data->sPlayer_rope_climb  = sprite_create(pointers, "sPlayer_rope_climb.bmp", 2, 10); 
        data->sPlayer_rope_slide  = sprite_create(pointers, "sPlayer_rope_slide.bmp", 2, 10); 
        data->sPlayer_splat_slow  = sprite_create(pointers, "sPlayer_splat_slow.bmp", 5, 7); 
        data->sPlayer_splat_swift = sprite_create(pointers, "sPlayer_splat_swift.bmp", 6, 10);
        data->sPlayer_turn        = sprite_create(pointers, "sPlayer_turn.bmp", 1, 6);       
        data->sPlayer_walk        = sprite_create(pointers, "sPlayer_walk.bmp", 4, 10);       
        data->sPlayer_walk_reach  = sprite_create(pointers, "sPlayer_walk_reach.bmp", 4, 10); 
        data->sPlayer_wire_idle   = sprite_create(pointers, "sPlayer_idle.bmp", 5, 6);  
        data->sPlayer_wire_walk   = sprite_create(pointers, "sPlayer_walk.bmp", 4, 6);  

        
    #define BMP_LOAD(name) data->##name = DEBUG_load_bmp(pointers, #name ".bmp")
        BMP_LOAD(sTest);
        BMP_LOAD(sTest_wide);
        BMP_LOAD(sMan);
        BMP_LOAD(sMan_anim);
        BMP_LOAD(sFont_test);
        BMP_LOAD(sFont_ASCII_lilliput);

        //LEVEL TEMP
        player_create(pointers, {BASE_W/2, BASE_H/2 - 20});
        wall_create(pointers, {Tile(3), Tile(20)}, {Tile(18), Tile(1)});
        wall_create(pointers, {Tile(17), Tile(20)}, {Tile(50), Tile(1)});
        wall_create(pointers, {Tile(4), Tile(11)}, {Tile(18), Tile(1)});
        wall_create(pointers, {Tile(2), Tile(11)}, {Tile(1), Tile(10)});
        wall_create(pointers, {Tile(32), Tile(11)}, {Tile(1), Tile(10)});
        // enemy_create(pointers, {10, 10});
    }
    
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
        pointers->data->camera_pos = player->pos - Vec2{BASE_W/2, BASE_H/2};
        enemy_update(pointers);
    }


    //DEBUG
    if (input.reset){  //NOTE: temp RESET
        player->pos = {BASE_W/2, BASE_H/2};
        player->spd = {};
    }

    GAME_DATA->draw_mode = Draw_Mode::world;
    draw_line(game_pointers, {40, 40}, {80, 80}, PURPLE);
    wall_draw(pointers);
    enemy_draw(pointers);
    player_draw(player, pointers);
    
    GAME_DATA->draw_mode = Draw_Mode::gui;
    char buffer[256];
    float32 zoom_scale = pointers->settings->zoom_scale;
    sprintf_s(buffer, "zoom: %.2f", zoom_scale);
    draw_text(pointers, buffer, {10, 10}, {1.f, 1.f});

    Vec2 mouse_gui_pos = (input.mouse_pos);    
    draw_rect(pointers, mouse_gui_pos, {2, 2}, MAGENTA);
    draw_rect(pointers, mouse_gui_pos, {1, 1}, LIME);
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
