//TODO: factor out c standard library
#include <math.h>  //TODO: intrinsics
#include <stdio.h> //using sprintf
#include <stdarg.h>


#include "my_types_constants.h"
#include "my_math.cpp"
#include "my_string.cpp"
#include "my_color.cpp"
#include "array_functions.h"

#include "game.h"

#include "render.cpp"
#include "collide.cpp"
#include "player.cpp"
#include "entity.cpp"
#include "text.cpp"



//TODO: move to debug file?
void debug_message_send(const char* fmt, ...)
{
    Debug_Message msg = {};
    int32 lifetime = DEBUG_MESSAGE_LIFETIME_DEFAULT;
    msg.lifetime = lifetime;

    char text_buffer[BUFF_LEN];
    va_list args;
    va_start(args, fmt);
    vsprintf(text_buffer, fmt, args);
    va_end(args);

    // msg.text = text_buffer;
    string_append(msg.text, text_buffer);
    
    pointers->data->debug_msg.current_message = msg;
    pointers->data->debug_msg.is_active = true;
}
void debug_message_update()
{
    Debug_Message_Queue* queue = &pointers->data->debug_msg;
    if (queue->is_active)
    {
        Debug_Message* msg = &queue->current_message;
        if (msg->lifetime-- <= 0)
        {
            queue->is_active = false;
        }

        draw_text(msg->text, queue->gui_pos);
    }
}





inline Vec2f
mouse_get_pos_gui()
{
    Vec2f result = pointers->input->mouse_pos_gui;
    return result;
}
inline Vec2f
mouse_get_pos_world()
{
    Vec2f result =
        {pointers->input->mouse_pos_gui.x / GAME_SETTINGS->zoom_scale,
         pointers->input->mouse_pos_gui.y / GAME_SETTINGS->zoom_scale};
    result += GAME_DATA->camera_pos;
    
    return result;
}
void
camera_zoom()
{
    auto input = *pointers->input;
    
    if (input.mouse_scroll > 0)
    {
        Vec2f mouse_prev = mouse_get_pos_world();

        if (input.shift.hold) GAME_SETTINGS->zoom_scale *= (float32)(sqrt(2));
        else GAME_SETTINGS->zoom_scale *= (float32)(sqrt(sqrt(2)));        
        Vec2f mouse_new = mouse_get_pos_world();
        Vec2f camera_pos_current = GAME_DATA->camera_pos;
        GAME_DATA->camera_pos =
            {camera_pos_current.x + mouse_prev.x - mouse_new.x,
             camera_pos_current.y + mouse_prev.y - mouse_new.y};
    }
    else if (input.mouse_scroll < 0)
    {
        Vec2f mouse_prev = mouse_get_pos_world();

        if (input.shift.hold) GAME_SETTINGS->zoom_scale /= (float32)(sqrt(2));
        else GAME_SETTINGS->zoom_scale /= (float32)(sqrt(sqrt(2)));
        Vec2f mouse_new = mouse_get_pos_world();
        Vec2f camera_pos_current = GAME_DATA->camera_pos;
        GAME_DATA->camera_pos =
            {camera_pos_current.x + mouse_prev.x - mouse_new.x,
             camera_pos_current.y + mouse_prev.y - mouse_new.y};

    }
    else if (input.mouse_front)
    {
        Vec2f mouse_prev = mouse_get_pos_world();
        GAME_SETTINGS->zoom_scale = 1;
        Vec2f mouse_new = mouse_get_pos_world();
        Vec2f camera_pos_current = GAME_DATA->camera_pos;
        GAME_DATA->camera_pos =
            {camera_pos_current.x + mouse_prev.x - mouse_new.x,
             camera_pos_current.y + mouse_prev.y - mouse_new.y};
    }    
}



////////UI STUFF/////////
//im_gui
globalvar int32 global_imgui_hot = -1;
globalvar int32 global_imgui_active = -1;
globalvar int32 global_imgui_num = 0;
void im_begin()
{
    global_imgui_hot = -1;
    global_imgui_num = 0;
}
void im_end()
{
    //TODO:
}
bool32 im_button(Vec2f button_pos, Vec2f button_size)//, int32 id)
{
    bool32 result = false;
    
    int32 id = global_imgui_num++;
    auto input = pointers->input;
    auto data = pointers->data;

    //mouse
    Vec2f mouse_pos = mouse_get_pos_gui();

    Color color = MAGENTA;
    bool32 is_hovered = collide_pixel_rect(mouse_pos, button_pos, button_size);
    bool32 is_hot = (global_imgui_hot == id);
    bool32 is_active = (global_imgui_active == id);
    
    //button logic
    if (is_hovered){
        global_imgui_hot = id;
        color = GRAY;

        if (input->mouse_left.press){
            global_imgui_active = id;
        }
    }
    if (is_active){
        color = RED;
        if (input->mouse_left.release){
            if (is_hovered) result = true;
            global_imgui_active = -1;
        }
    }
    
    draw_rect(button_pos, button_size, color);
    
    return result;
}




//Editor
globalvar Ent_Type global_editor_entity_to_spawn = Ent_Type::Player;
globalvar Entity* global_editor_selected_entity = nullptr;

Entity*
entity_spawn(Ent_Type type, Vec2f pos)
{
    Entity* result = nullptr;

    switch (type)
    {
        case Ent_Type::Player:         player_create(pos); break;
        case Ent_Type::Enemy: result = enemy_create(pos); break;
        case Ent_Type::Wall:  result = wall_create(pos, {Tile(1), Tile(1)}); break;
    }

    return result;
}


Entity* editor_select_entity()
{
    Entity* result = nullptr;
    Entity* array = pointers->entity->array;
    for (int ent_index = 0; ent_index < ENT_MAX; ++ent_index)
    {
        Entity* ent = &array[ent_index];
        V2f pos = mouse_get_pos_world();
        if (collide_pixel_rect(pos, ent->pos, ent->size))
            result = ent;
    }
    return result;
}
void editor_draw_selected()
{
    Entity* ent_hl = global_editor_selected_entity;
    if (!ent_hl) return;
    
    int32 alpha = 60;
    
    Color color = PINK;
    color.a = (u8)alpha;
    Rect rect = {
        ent_hl->pos,
        ent_hl->size
    };
    draw_rect(rect, color);
}

void
editor_save_scene(const char* filename)
{
    debug_message_send("Level Saved: \"%s\"", filename);
        
    char buffer[256];
    
    Entity* array = pointers->entity->array;
    for (int entity_index = 0; entity_index < ENT_MAX; ++entity_index)
    {
        Entity* entity = &array[entity_index];
        if (entity->is_alive){
            char ent_string[128];
            sprintf(ent_string, "e%i x%i y%i w%i h%i \n",
                    (int32)entity->type,
                    (int32)entity->pos.x,
                    (int32)entity->pos.y,
                    (int32)entity->size.x,
                    (int32)entity->size.y
            );
            
            //append
            string_cat(buffer, buffer, ent_string);
            
        }
    }

    string_append(buffer, "!");
    
    pointers->memory->DEBUG_platform_file_write_entire(filename, string_length(buffer), &buffer);
}
void editor_load_scene(const char* filename)
{
    debug_message_send("Level Loaded: \"%s\"", filename);
    
    //clear entities
    // *pointers->entity = {};
    entity_clear_all();
    // pointers->entity->nums;
        
    player_create({40, 40}); //HACKS:
    // pointers->entity->names = global_ent_names; //TODO: reset or avoid clearning names
    
    //load file
    DEBUG_File file = pointers->memory->DEBUG_platform_file_read_entire(filename);
    char* file_mem = (char*)file.memory;
    
    Ent_Type type = Ent_Type::Null;
    Vec2f pos = {};
    Vec2f size = {};
    char value_string[64];

    uint32 beginning_of_statement_index = 0;
    uint32 end_of_statement_index = 0;
    for (uint32 char_index = 0; char_index < LOOP_MAX; ++char_index)
    {
        char c = file_mem[char_index];

        if (c == ' ') //end of statement
        {
            end_of_statement_index = char_index;
            
            //get value
            u32 value_length = end_of_statement_index - beginning_of_statement_index;
            for (u32 statement_index = 0;
                 statement_index < value_length;
                 ++statement_index)
            {
                char cs = file_mem[statement_index + beginning_of_statement_index];
                value_string[statement_index] = cs;
            }
            value_string[value_length] = 0; //null terminate string 
            i32 value = string_get_i32(value_string);
            
            //get which value it is
            auto var = file_mem[beginning_of_statement_index];
            switch (var)
            {
                case 'e': type   = (Ent_Type)value; break;
                case 'x': pos.x  = (f32)value; break;
                case 'y': pos.y  = (f32)value; break;
                case 'w': size.x = (f32)value; break;
                case 'h': size.y = (f32)value; break;
            }            
            
            beginning_of_statement_index = char_index+1;

        }        
        else if (c == '\n') //end of line
        {
            Entity* entity_spawned = entity_spawn(type, pos);
            entity_spawned->size = size;
        }
        else if (c == '!') //end of file
        {
            break;
        }
    }
    //post-loop
}





extern "C" GAME_INPUT_CHANGE_DEVICE(game_input_change_device)
{ //TODO: this should most likely be platform code
    *input_map = {};
    input_map->game_input_device = input_device_current;
}




void //TODO: CLEAN UP
game_initialize(Game_Pointers* _game_pointers)
{
    // game_pointers = _game_pointers;
    pointers->memory->is_initalized = true;
    
    Game_Data*      data     = pointers->data;
    Game_Entities*  entity   = &data->entity;
    Game_Sprites*   sprite   = &data->sprites;
    Player*         player   = &entity->player;
    Game_Settings*  settings = pointers->settings;
    Game_Input_Map* input    = pointers->input;

    //add Entity names into Data struct
    for (int i = 0; i < array_length(entity->names); ++i)
    {
        entity->names[i] = global_ent_names[i];
    }
    
    Assert( (&input->bottom_button - &input->buttons[0]) == (array_length(input->buttons) - 1) );
    Assert( sizeof(Game_Data) <= pointers->memory->permanent_storage_space );
    Assert( (&entity->bottom_entity - &entity->array[0]) == (array_length(entity->array) - 1) );

    data->debug_msg.gui_pos = DEBUG_MESSAGE_POS_DEFAULT;

    data->state = GAME_STATE_DEFAULT;
    data->draw_mode = GAME_DRAW_MODE_DEFAULT;
    
    data->camera_yoffset_extra = 4.f;
    data->camera_pos_offset_default = {
        BASE_W/2,
        BASE_H/2 + pointers->data->camera_yoffset_extra};
    data->camera_pos_offset = pointers->data->camera_pos_offset_default;

#define PLR_SPR_ORIGIN {6.f, 5.f}
  //IMAGES
    sprite->sPlayer_air         = sprite_create("sPlayer_air2.bmp", 7, 15, PLR_SPR_ORIGIN);
    sprite->sPlayer_air_reach   = sprite_create("sPlayer_air_reach.bmp", 2, 0, PLR_SPR_ORIGIN);  
    sprite->sPlayer_idle        = sprite_create("sPlayer_idle.bmp", 2, 0, PLR_SPR_ORIGIN);               
    sprite->sPlayer_ledge       = sprite_create("sPlayer_ledge.bmp", 4, 10, PLR_SPR_ORIGIN);
    sprite->sPlayer_ledge_reach = sprite_create("sPlayer_ledge_reach.bmp", 3, 0, PLR_SPR_ORIGIN);
    sprite->sPlayer_rope_climb  = sprite_create("sPlayer_rope_climb.bmp", 2, 10, PLR_SPR_ORIGIN); 
    sprite->sPlayer_rope_slide  = sprite_create("sPlayer_rope_slide.bmp", 2, 10, PLR_SPR_ORIGIN); 
    sprite->sPlayer_splat_slow  = sprite_create("sPlayer_splat_slow.bmp", 5, 7, PLR_SPR_ORIGIN); 
    sprite->sPlayer_splat_swift = sprite_create("sPlayer_splat_swift.bmp", 6, 10, PLR_SPR_ORIGIN);
    sprite->sPlayer_turn        = sprite_create("sPlayer_turn.bmp", 1, 6, PLR_SPR_ORIGIN);
    sprite->sPlayer_walk        = sprite_create("sPlayer_walk.bmp", 4, 10, PLR_SPR_ORIGIN);
    sprite->sPlayer_walk_reach  = sprite_create("sPlayer_walk_reach.bmp", 4, 10, PLR_SPR_ORIGIN);
    sprite->sPlayer_wire_idle   = sprite_create("sPlayer_idle.bmp", 5, 6, PLR_SPR_ORIGIN);
    sprite->sPlayer_wire_walk   = sprite_create("sPlayer_walk.bmp", 4, 6, PLR_SPR_ORIGIN);

    
    sprite->sMouse_cursors   = sprite_create("sMouse_cursors2.bmp", 4, 0, {2, 2});

        
#define BMP_LOAD(name) data->##name = DEBUG_load_bmp(#name ".bmp")
    BMP_LOAD(sTest);
    BMP_LOAD(sTest_wide);
    BMP_LOAD(sMan);
    BMP_LOAD(sMan_anim);
    BMP_LOAD(sFont_test);
    BMP_LOAD(sFont_ASCII_lilliput);
    BMP_LOAD(sFont_ASCII_lilliput_vert);

    font_create(&data->sFont_ASCII_lilliput_vert, 8);
    
  //LEVEL TEMP
    player_create({BASE_W/2, BASE_H/2 - 20});
    wall_create({Tile(3), Tile(20)}, {Tile(18), Tile(1)});
    wall_create({Tile(17), Tile(20)}, {Tile(50), Tile(1)});
    wall_create({Tile(4), Tile(11)}, {Tile(18), Tile(1)});
    wall_create({Tile(2), Tile(11)}, {Tile(1), Tile(10)});
    wall_create({Tile(32), Tile(11)}, {Tile(1), Tile(10)});
}




//UPDATE
extern "C" GAME_UPDATE_AND_DRAW(game_update_and_draw)
{
    pointers = __game_pointers;

    if (!pointers->memory->is_initalized){
        //TODO: zero out the necessary data before calling game_initialize so we can reset the game by setting "is_initialized" to false
        game_initialize(pointers);
    }

    Game_Settings* settings = pointers->settings;
    Game_Data*     data     = pointers->data;
    Game_Sprites*  sprite   = &data->sprites;
    Game_Entities* entity   = &data->entity;
    Player*        player   = &entity->player;

    //UPDATE (early)
    camera_zoom();
    if (input.reset)
        player->pos = {BASE_W/2, 0};
    if (input.debug_mode_toggle)
        data->debug_mode_enabled = !data->debug_mode_enabled;

    

    
    //STATE UPDATE
    if (data->state == Game_State::Edit)
    {
        if (input.mouse_left)
            global_editor_selected_entity = editor_select_entity();
        if (input.editor_toggle)
            data->state = Game_State::Play;
        if (input.ctrl.hold && input.editor_save_level)
            editor_save_scene("testy_too.lvl");
        if (input.ctrl.hold && input.editor_load_level)
            editor_load_scene("testy_too.lvl");

      //Entities
        if (input.mouse_right.release)
        {
            Vec2f mouse_pos_world = mouse_get_pos_world();
            Vec2f pos_spawn = {round_f32(mouse_pos_world.x), round_f32(mouse_pos_world.y)};
            entity_spawn(global_editor_entity_to_spawn, pos_spawn);
        }

        Entity* entity_under_mouse = collide_pixel_get_any_entity(mouse_get_pos_world());
        if (entity_under_mouse)
        {
            if (entity_under_mouse->is_alive)
                if (input.jump)
                    entity_destroy(entity_under_mouse);
        }

      //Camera
        float32 cam_speed = (input.shift.hold ? 4.f : 2.f);
        Vec2f move_input =
            {(float32)(input.right.hold - input.left.hold) / GAME_SETTINGS->zoom_scale,
             (float32)(input.down.hold - input.up.hold) / GAME_SETTINGS->zoom_scale};
        data->camera_pos += move_input * Vec2f{cam_speed, cam_speed};
        
      //Draw coords
        data->draw_mode = Draw_Mode::World;
        float32 static_size = game_get_static_size(0.5);
        draw_rect({0, 0}, {static_size, 999999.f}, WHITE);
        draw_rect({0, 0}, {999999.f, static_size}, WHITE);

      //Draw grid //TODO: make straight line functions for this use case
        int32 grid_spacing = TILE_SIZE;
        float32 grid_line_size = (1.f/pointers->settings->window_scale) * static_size;
        Color color_faded = {255, 255, 255, 50};
        for (int grid_x = 0; grid_x < 1000; ++grid_x)
        {
            if (grid_x - data->camera_pos.x > BASE_W) break;
            draw_rect({(float32)grid_x * grid_spacing, 0}, {grid_line_size, 1000}, color_faded);
        }
        for (int grid_y = 0; grid_y < 1000; ++grid_y)
        {
            if (grid_y - data->camera_pos.y > BASE_H) break;
            draw_rect({0, (float32)grid_y * grid_spacing}, {1000, grid_line_size}, color_faded);
        }
        
    }
    else if (data->state == Game_State::Play)
    {
        //controls
        if (input.editor_toggle){
            data->state = Game_State::Edit;
        }

        //entity update
        player_update(input);
        enemy_update();

        //camera update
        float32 cam_yoffset_extra = 4;
        Vec2f vec2_zoom = {GAME_SETTINGS->zoom_scale, GAME_SETTINGS->zoom_scale};
        data->camera_pos_offset = data->camera_pos_offset_default / vec2_zoom;
        data->camera_pos = player->pos - data->camera_pos_offset;
    }

    data->draw_mode = Draw_Mode::World;
    {
        wall_draw();
        enemy_draw();
        player_draw(player);
        
        editor_draw_selected();
    }
    
    data->draw_mode = Draw_Mode::Gui;
    {
        im_begin();
        {
            Vec2f button_pos = {10, 40};
            Vec2f button_size = {16, 6};
            draw_text_buffer(
                {button_pos.x, button_pos.y - button_size.y}, {0.5f, 0.5f}, {5, 8},
                "selected:%s", entity_get_name(global_editor_entity_to_spawn)
            );

            for (int ent_index = 0; ent_index < (int32)Ent_Type::Num; ++ent_index)
            {
                Vec2f final_pos = {button_pos.x, button_pos.y + ((button_size.y + 1) * ent_index)};
                if (im_button(final_pos, button_size))
                    global_editor_entity_to_spawn = (Ent_Type)ent_index;
                draw_text(global_ent_names[ent_index], final_pos, {0.5, 0.5});
            }
        }
        im_end();
    

        //debug
        debug_message_update();
        IF_DEBUG
        {
            //draw performance
            auto perf = pointers->performance;
            char perf_string[256];
            sprintf_s(perf_string,
                      "ms/f: %.2f" "\n"
                      "mc/f: %.2f" "\n"
                      ,perf->ms_frame
                      ,perf->megacycles_frame
            );
            draw_text(perf_string, {0, 0}, {0.5f, 0.5f}, {7, 8});
    
        }
    
        //draw mouse
        sprite->sMouse_cursors.origin = {2, 2};
        draw_sprite_anim(&sprite->sMouse_cursors, input.mouse_pos_gui, 0);
        // draw_rect(input.mouse_pos_gui + Vec2f{1, 1}, {2, 2}, MAGENTA);
        // draw_rect(input.mouse_pos_gui, {1, 1}, LIME);
    }
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



// void
// draw_text_old(const char* text, Vec2f pos, Vec2f scale = {1, 1}, Vec2 spacing = {5, 8})
// {
//     BMP_File* font_image = game_pointers->data->sFont_ASCII_lilliput;
//     int32 frame_size = font_image->height;
//     float32 drawx_offset = 0; //exists for us to handle the position simply
//     int32 loop_length = string_length(text);
//     for (int i = 0; i < loop_length; ++i)
//     {
//         char frame = text[i];
//         if (frame == '\n')
//         {
//             pos.y += (spacing.y * scale.y);
//             drawx_offset = 0;
//             continue;
//         }

//         char frame_offset = frame - FONT_ASCII_CHARACTER_START_OFFSET;
//         draw_bmp_part(font_image,
//                       {pos.x + drawx_offset, pos.y}, //area_pos
//                       {scale.x, scale.y},            //overall scale
//                       frame_offset * frame_size, 0, //pos on image
//                       frame_size, frame_size);      //size to draw
        
//         drawx_offset += (spacing.x * scale.x);
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
