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
void editor_entity_spawn(Ent_Type type, Vec2 pos, Game_Entities* entity)
{
    if (type == Ent_Type::player)     entity->player.Create(pos);
    else if (type == Ent_Type::enemy) entity->enemys.Create(pos);
    else if (type == Ent_Type::wall)  entity->walls.Create(pos, {Tile(1), Tile(1)});
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
    
    //initialize
    if (!game_pointers->memory->is_initalized)
    {
        Assert( (&input.bottom_button - &input.buttons[0]) == (array_length(input.buttons) - 1) );
        Assert( sizeof(Game_Data) <= game_pointers->memory->permanent_storage_space );

        data->state = Game_State::play;
        game_pointers->data = (Game_Data*)game_pointers->memory->permanent_storage;
        game_pointers->entity = &game_pointers->data->entity;
        game_pointers->memory->is_initalized = true;

        DEBUG_level_load(LEVEL_FIRST, *game_pointers);

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

        if (input.ctrl.hold)
        {
            if (input.editor_save_level)
                DEBUG_level_save("test.lvl", *game_pointers);
            if (input.editor_load_level)
                DEBUG_level_load("test.lvl", *game_pointers);
        }

            
        global_editor_ent_to_spawn = editor_pick_entity(input);
        if (input.left_click)
            editor_entity_spawn(global_editor_ent_to_spawn, input.mouse_pos, entity);        
    }
    else if (data->state == Game_State::play)
    {
        if (input.editor_toggle){
            data->state = Game_State::edit;
            DEBUG_level_load(LEVEL_FIRST, *game_pointers);
        }
            
        player->Update(*game_pointers, input);
        enemys->Update(*game_pointers);
        bullets->Update(*game_pointers);
    }


    //DEBUG
    if (input.reset)  //NOTE: temp RESET
        DEBUG_level_load(data->level_current, *game_pointers);

    
    //draw
    walls->Draw(*game_pointers);
    enemys->Draw(*game_pointers);
    bullets->Draw(*game_pointers);
    player->Draw(*game_pointers);

    persist float32 draw_y = 0;
    persist float32 draw_x = 0;
    if (input.jump) draw_x -= 0.05f;
    draw_bmp(*game_pointers, data->bmp_file, {draw_x, draw_y});    
    
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
