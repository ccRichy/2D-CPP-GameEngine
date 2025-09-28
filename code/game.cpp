#include "my_math.cpp"
#include "my_string.cpp"
#include "my_color.h"
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
    //TODO: error handle
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

globalvar Ent_Type global_editor_ent_to_spawn; //NOTE: factor out
Ent_Type editor_pick_entity(Game_Input_Map input)
{
    auto return_val = global_editor_ent_to_spawn;
    if (input.num1)      return_val = Ent_Type::player;
    else if (input.num2) return_val = Ent_Type::enemy;
    else if (input.num3) return_val = Ent_Type::wall;
    return return_val;
}
void editor_entity_spawn(Ent_Type type, Vec2 pos, Game_Entities* entity)
{
    if (type == Ent_Type::player)     entity->player.Create(pos);
    else if (type == Ent_Type::enemy) entity->enemys.Create(pos);
    else if (type == Ent_Type::wall)  entity->walls.Create(pos, {Tile(1), Tile(1)});
}



BMP_File*
DEBUG_load_bmp(Game_Pointers game_pointers, const char* filename)
{
    return (BMP_File*)game_pointers.memory->DEBUG_platform_file_read_entire(filename).memory;
};

void
draw_bmp(Game_Pointers game_pointers, BMP_File* bmp, Vec2 pos)
{
    Game_Render_Buffer* render = game_pointers.render;
    int8 scale = game_pointers.settings->window_scale;
    
    pos.y += bmp->height;
    int32 x_start = round_i32(pos.x * scale);
    int32 x_end   = x_start + (bmp->width * scale);
    int32 x_offscreen_amt = 0;
    int32 y_start = round_i32(pos.y * scale) + (scale-1);
    int32 y_end   = y_start - (bmp->height * scale);
    
    if (x_start < 0){
        x_offscreen_amt = -x_start; //NOTE: flipping sign
        x_start = 0;
    }
    if (x_end > render->width) x_end      = render->width;
    if (y_end < 0) y_end                  = 0;
    if (y_start > render->height) y_start = render->height;
    
    int32 pixel_offscreen_amt = x_offscreen_amt % scale;
    int32 pixel_increment = 0;
    int32 row_increment = 0;

    uint8* row = (uint8*)render->memory + (y_start * render->pitch);
    uint32* bitmap_pixel = (uint32*)((uint8*)bmp + bmp->offset);
    int32 bitmap_pixel_offset = (int32)(x_offscreen_amt / scale);
    bitmap_pixel += bitmap_pixel_offset;

    for (int Y = y_end; Y < y_start; ++Y)
    {
        uint32* pixel = ((uint32*)row + x_start);
        for (int X = x_start; X < x_end; ++X)
        {
            float32 alpha = (float32)(*bitmap_pixel >> 24 & 0xFF) / 255.f;
            float32 alphasub = 1 - (alpha);

            uint32 color_prev = *pixel;
            uint32 color_new = *bitmap_pixel;

            uint32 target_color = (
                color_channel_get_transparent((color_prev >> 16 & 0xFF), (color_new >> 16 & 0xFF), alpha) << 16 |
                color_channel_get_transparent((color_prev >> 8 & 0xFF), (color_new >> 8 & 0xFF), alpha) << 8 |
                color_channel_get_transparent((color_prev & 0xFF), (color_new & 0xFF), alpha)
            );

            *pixel++ = target_color;

            if (X == x_start)
            {
                pixel_increment = pixel_offscreen_amt + 1;
            }
            else if (++pixel_increment >= scale)
            {
                pixel_increment = 0;
                bitmap_pixel++;
            }
        }

        if (++row_increment >= scale)
        {
            bitmap_pixel += bmp->width;
            row_increment = 0;
        }
        bitmap_pixel -= (bmp->width - (int32)((float32)x_offscreen_amt / scale));
        row -= render->pitch;
    }
}

void
draw_bmp_old(Game_Pointers game_pointers, BMP_File* bmp, Vec2 pos)
{
    pos.y += bmp->height;
    uint32* bitmap_pixel = (uint32*)((uint8*)bmp + bmp->offset);
    
    for (int pixel_index = 0; pixel_index < bmp->width * bmp->height; ++pixel_index)
    {
        Color color_cast = *(Color*)bitmap_pixel;
        Color color_converted = {color_cast.b, color_cast.g, color_cast.r, color_cast.a};

        int32 xoffset = pixel_index % bmp->width;
        float32 draw_x = pos.x + xoffset;
        draw_pixel(game_pointers, {draw_x, pos.y}, color_converted);

        if (xoffset >= bmp->width-1)
            pos.y -= 1;
        bitmap_pixel++;
    }
}



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
    
    //initialize
    if (!game_pointers->memory->is_initalized)
    {
        Assert( (&input.bottom_button - &input.buttons[0]) == (array_length(input.buttons) - 1) );
        Assert( sizeof(Game_Data) <= game_pointers->memory->permanent_storage_space );

        data->state = Game_State::play;
        game_pointers->data = (Game_Data*)game_pointers->memory->permanent_storage;
        game_pointers->entity = &game_pointers->data->entity;
        game_pointers->memory->is_initalized = true;

        level_load(LEVEL_FIRST, *game_pointers);

        //bitmap test
        // bitmap_file = (BMP_File*)game_pointers->memory->DEBUG_platform_file_read_entire("bitmap.bmp").memory;
        data->bmp_file = DEBUG_load_bmp(*game_pointers, "bitmap.bmp");
    }


    // DEBUG_draw_bmp(*game_pointers, data->bmp_file, {50, 50});    
    Player*  player  = &entity->player;
    Walls*   walls   = &entity->walls;
    Enemys*  enemys  = &entity->enemys;
    Bullets* bullets = &entity->bullets;

    if (data->state == Game_State::edit)
    {
        if (input.editor_toggle)
            data->state = Game_State::play;
            
        global_editor_ent_to_spawn = editor_pick_entity(input);
        if (input.left_click)
            editor_entity_spawn(global_editor_ent_to_spawn, input.mouse_pos, entity);        
    }
    else if (data->state == Game_State::play)
    {
        if (input.editor_toggle){
            data->state = Game_State::edit;
            level_load(LEVEL_FIRST, *game_pointers);
        }
            
        player->Update(*game_pointers, input);
        enemys->Update(*game_pointers);
        bullets->Update(*game_pointers);
    }


    //DEBUG
    if (input.reset)  //NOTE: temp RESET
        level_load(data->level_current, *game_pointers);

    if (input.ctrl.hold)
    {
        if (input.editor_save_level)
            level_save("test.lvl", *game_pointers);
        if (input.editor_load_level)
            level_load("test.lvl", *game_pointers);
    }
    
    //draw
    walls->Draw(*game_pointers);
    enemys->Draw(*game_pointers);
    bullets->Draw(*game_pointers);
    player->Draw(*game_pointers);

    persist float32 draw_y = 10;
    persist float32 draw_x = -2.1f;
    if (input.jump) draw_x -= 0.05f;
    // draw_y -= 0.1f;
    // draw_x -= 0.01f;
    draw_bmp(*game_pointers, data->bmp_file, {draw_x, draw_y});

    // Color red_a = {255, 0, 0, 100};
    // Color green_a = {0, 255, 0, 100};
    // draw_rect(*game_pointers, {40, 40}, {40, 40}, red_a);
    // draw_rect(*game_pointers, {60, 60}, {40, 40}, green_a);
    
    
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
