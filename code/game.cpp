//TODO: FACTOR OUT C STANDARD LIBRARY
#include "math.h"  //TODO: INTRINSICS
#include "stdio.h" //using sprintf

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


//static Game_Pointers* pointers;


inline Vec2
mouse_get_pos_gui()
{
    Vec2 result = pointers->input->mouse_pos_gui;
    return result;
}
inline Vec2
mouse_get_pos_world()
{
    Vec2 result =
        {pointers->input->mouse_pos_gui.x / GAME_SETTINGS->zoom_scale,
        pointers->input->mouse_pos_gui.y / GAME_SETTINGS->zoom_scale};
    
    result += GAME_DATA->camera_pos;
    return result;
}


BMP_Data
DEBUG_load_bmp(const char* filename)
{
    auto file = (BMP_File_Header*)pointers->memory->DEBUG_platform_file_read_entire(filename).memory;
    BMP_Data result = {};
    result.size_bytes = file->size;
    result.width = file->width;
    result.height = file->height;
    result.pixels = (uint32*)((uint8*)file + file->offset);
    result.bits_per_pixel = file->bits_per_pixel;
        
    Assert(result.size_bytes > 0);
    return result;
};
Sprite
sprite_create(const char* bmp_filename, uint32 frame_num, float32 fps)
{
    Sprite sprite = {};   
    sprite.bmp = DEBUG_load_bmp(bmp_filename);
    sprite.origin = {};
    sprite.fps = fps;
    sprite.frame_num = frame_num;
    sprite.is_animation = (frame_num > 0 && fps != 0);
    return sprite;
}



/*
  NOTE:
  Font assumes a !vertical image! so we can crawl linearly over the source data
 */
Font
font_create(BMP_Data* image, uint32 glyph_size)
{
    Font result = {};

    uint32 glyph_area = glyph_size * glyph_size;
    uint32 image_area = image->width * image->height;
    
    int32 glyph_offset = FONT_LENGTH;
    int32 glyph_size_bytes = glyph_area * (image->bits_per_pixel / 8);
    uint32* image_pixels = image->pixels;

    //loop over the entire image & move image pixels into respective glyph pixels
    uint32 glyph_ticker = 0;
    for (uint32 image_pixel_index = 0; image_pixel_index < image_area; ++image_pixel_index)
    {
        uint32 image_pixel = image_pixels[image_pixel_index];
        Color* DEBUG_color = (Color*)(&image_pixel);
        Glyph* glyph = &pointers->data->font_test.glyphs[glyph_offset];
        
        int32 glyph_x = image_pixel_index % glyph_size;
        //reverse vertical pixel arrangement
        int32 glyph_y = glyph_size-1 - floor_i32((float32)(image_pixel_index % glyph_area) / image->width);
        
        glyph->pixels[glyph_y][glyph_x] = image_pixel;

        if (glyph_offset <= 0)
            return result;
        if (++glyph_ticker >= glyph_area){
            glyph_offset--;
            glyph_ticker = 0;
        }
    }
    
    return result;
}
void
draw_glyph(char text_character, Vec2 pos, Vec2 scale)
{
    auto render = pointers->render;
    int32 glyph_size = 8;

    float32 draw_scale = game_get_draw_scale();
    float32 xscale_final = abs_f32(draw_scale * scale.x);
    float32 yscale_final = abs_f32(draw_scale * scale.y);
    pos               += game_get_draw_pos();


    int32 x_left   = round_i32(pos.x * draw_scale);
    int32 x_right  = round_i32(x_left + (glyph_size * xscale_final));
    int32 y_top    = round_i32(pos.y * draw_scale);
    int32 y_bottom = round_i32(y_top + (glyph_size * yscale_final));

    if (x_left < 0)                 x_left = 0;
    if (x_right > render->width)    x_right = render->width;
    if (y_top < 0)                  y_top = 0;
    if (y_bottom > render->height)  y_bottom = render->height;

    float32 render_pos_yoffset = pos.y;
    uint32* render_pixel = (uint32*)(render->memory);
    render_pixel += (int32)(render_pos_yoffset * render->pitch);
    Glyph* glyph = &pointers->data->font_test.glyphs[text_character - FONT_ASCII_CHARACTER_START_OFFSET];
    //loop through all real coords
    int32 image_pixel_x = 0;
    int32 image_pixel_y = 0;
    for (int32 Y_Index = y_top; Y_Index < y_bottom; ++Y_Index)
    {
        render_pixel = (uint32*)((uint8*)render->memory + (Y_Index * render->pitch));
        
        for (int X_Index = x_left; X_Index < x_right; ++X_Index)
        {
            image_pixel_x = (int32)((X_Index - x_left) / xscale_final);
            image_pixel_y = (int32)((Y_Index - y_top) / yscale_final);

            uint32 color_prev = render_pixel[X_Index];
            uint32 color = glyph->pixels[image_pixel_y][image_pixel_x];

            //send color to buffer
            if (color > 0)
                render_pixel[X_Index] = color;
        }
    }
}
void draw_text(const char* text, Vec2 pos, Vec2 scale = {1, 1}, Vec2 spacing = {5, 8})
{
    Vec2 final_pos = pos;
    int32 text_length = string_length(text);
    int32 new_lines = 0;
    for (int i = 0; i < text_length; ++i)
    {
        char char_to_draw = text[i];
        if (char_to_draw == '\n')
        {
            new_lines++;
            final_pos.x = pos.x;
            final_pos.y += (spacing.y * scale.y);
        }
        else
        {
            draw_glyph(char_to_draw, final_pos, scale);
            final_pos.x += (spacing.x * scale.x);
        }
    }
}



////////UI STUFF/////////
//im_gui
globalvar int32 global_imgui_hot;
globalvar int32 global_imgui_active;
//editor
globalvar Ent_Type global_editor_entity_to_spawn;

bool32 im_button(Vec2 button_pos, Vec2 button_size, int32 id)
{
    if (true) return false;
    bool32 result = false;
    
    auto input = pointers->input;
    auto data = pointers->data;

    //mouse
    Vec2 mouse_pos = {};
    if (data->draw_mode == Draw_Mode::gui)        mouse_pos = mouse_get_pos_gui();
    else if (data->draw_mode == Draw_Mode::world) mouse_pos = mouse_get_pos_world();

    //button logic
    Color color = MAGENTA;
    if (collide_pixel_rect(mouse_pos, button_pos, button_size)){
        global_imgui_hot = id;
        color = WHITE;
    }

    if (global_imgui_hot == id){
        if (input->mouse_left.press)
            global_imgui_active = id;
    }
    if (global_imgui_active == id){
        if (input->mouse_left.release)
            result = true;
    }
        

    draw_rect(button_pos, button_size, color);
    
    return result;
}

Entity*
editor_spawn_entity(Ent_Type type)
{
    Entity* result = nullptr;
    Vec2 mouse_pos = mouse_get_pos_world();
    Vec2 pos = {round_f32(mouse_pos.x), round_f32(mouse_pos.y)};
    
    if      (type == Ent_Type::player) player_create(pos);
    else if (type == Ent_Type::enemy)  result = enemy_create(pos);
    else if (type == Ent_Type::wall)   result = wall_create(pos, {Tile(1), Tile(1)});

    return result;
}



void
camera_zoom()
{
    auto input = *pointers->input;
    
    if (input.mouse_scroll > 0)
    {
        Vec2 mouse_prev = mouse_get_pos_world();

        if (input.shift.hold) GAME_SETTINGS->zoom_scale *= (float32)(sqrt(2));
        else GAME_SETTINGS->zoom_scale *= (float32)(sqrt(sqrt(2)));        
        Vec2 mouse_new = mouse_get_pos_world();
        Vec2 camera_pos_current = GAME_DATA->camera_pos;
        GAME_DATA->camera_pos =
            {camera_pos_current.x + mouse_prev.x - mouse_new.x,
             camera_pos_current.y + mouse_prev.y - mouse_new.y};
    }
    else if (input.mouse_scroll < 0)
    {
        Vec2 mouse_prev = mouse_get_pos_world();

        if (input.shift.hold) GAME_SETTINGS->zoom_scale /= (float32)(sqrt(2));
        else GAME_SETTINGS->zoom_scale /= (float32)(sqrt(sqrt(2)));
        Vec2 mouse_new = mouse_get_pos_world();
        Vec2 camera_pos_current = GAME_DATA->camera_pos;
        GAME_DATA->camera_pos =
            {camera_pos_current.x + mouse_prev.x - mouse_new.x,
             camera_pos_current.y + mouse_prev.y - mouse_new.y};

    }
    else if (input.mouse_front)
    {
        Vec2 mouse_prev = mouse_get_pos_world();
        GAME_SETTINGS->zoom_scale = 1;
        Vec2 mouse_new = mouse_get_pos_world();
        Vec2 camera_pos_current = GAME_DATA->camera_pos;
        GAME_DATA->camera_pos =
            {camera_pos_current.x + mouse_prev.x - mouse_new.x,
             camera_pos_current.y + mouse_prev.y - mouse_new.y};
    }    
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

    data->state = GAME_STATE_DEFAULT;
    data->draw_mode = GAME_DRAW_MODE_DEFAULT;
    
    data->camera_yoffset_extra = 4.f;
    data->camera_pos_offset_default = {
        BASE_W/2,
        BASE_H/2 + pointers->data->camera_yoffset_extra};
    data->camera_pos_offset = pointers->data->camera_pos_offset_default;    
    
  //IMAGES
    data->sPlayer_air         = sprite_create("sPlayer_air2.bmp", 7, 15);
    data->sPlayer_air_reach   = sprite_create("sPlayer_air_reach.bmp", 2, 0);  
    data->sPlayer_idle        = sprite_create("sPlayer_idle.bmp", 2, 0);               
    data->sPlayer_ledge       = sprite_create("sPlayer_ledge.bmp", 4, 10);      
    data->sPlayer_ledge_reach = sprite_create("sPlayer_ledge_reach.bmp", 3, 0);
    data->sPlayer_rope_climb  = sprite_create("sPlayer_rope_climb.bmp", 2, 10); 
    data->sPlayer_rope_slide  = sprite_create("sPlayer_rope_slide.bmp", 2, 10); 
    data->sPlayer_splat_slow  = sprite_create("sPlayer_splat_slow.bmp", 5, 7); 
    data->sPlayer_splat_swift = sprite_create("sPlayer_splat_swift.bmp", 6, 10);
    data->sPlayer_turn        = sprite_create("sPlayer_turn.bmp", 1, 6);       
    data->sPlayer_walk        = sprite_create("sPlayer_walk.bmp", 4, 10);       
    data->sPlayer_walk_reach  = sprite_create("sPlayer_walk_reach.bmp", 4, 10); 
    data->sPlayer_wire_idle   = sprite_create("sPlayer_idle.bmp", 5, 6);  
    data->sPlayer_wire_walk   = sprite_create("sPlayer_walk.bmp", 4, 6);  

        
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
        //TODO: zero out the necessary data before calling game_initialize so we can use this to reset the game
        game_initialize(pointers);
    }

    Game_Data* data = pointers->data;
    Game_Entities* entity = &data->entity;
    Game_Settings* settings = pointers->settings;
    Player* player = &entity->player;
    
    
    //UPDATE (early)
    camera_zoom();
    if (input.reset)
        player->pos = {BASE_W/2, 0};
    if (input.debug_mode_toggle)
        data->debug_mode_enabled = !data->debug_mode_enabled;

    
    //STATE UPDATE
    if (data->state == Game_State::edit)
    {
        if (input.editor_toggle)
            data->state = Game_State::play;

      //Entities
        if (input.mouse_right.release)
            editor_spawn_entity(global_editor_entity_to_spawn);

        Entity* entity_under_mouse = collide_pixel_get_any_entity(mouse_get_pos_world());
        if (entity_under_mouse)
        {
            if (entity_under_mouse->is_alive)
                if (input.jump)
                    entity_destroy(entity_under_mouse);
        }

      //Camera
        float32 cam_speed = (input.shift.hold ? 4.f : 2.f);
        Vec2 move_input =
            {(float32)(input.right.hold - input.left.hold) / GAME_SETTINGS->zoom_scale,
             (float32)(input.down.hold - input.up.hold) / GAME_SETTINGS->zoom_scale};
        data->camera_pos += move_input * Vec2{cam_speed, cam_speed};
        
      //Draw coords
        data->draw_mode = Draw_Mode::world;
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
    else if (data->state == Game_State::play)
    {
        //controls
        if (input.editor_toggle){
            data->state = Game_State::edit;
        }

        //entity update
        player_update(input);
        enemy_update();

        //camera update
        float32 cam_yoffset_extra = 4;
        Vec2 vec2_zoom = {GAME_SETTINGS->zoom_scale, GAME_SETTINGS->zoom_scale};
        data->camera_pos_offset = data->camera_pos_offset_default / vec2_zoom;
        data->camera_pos = player->pos - data->camera_pos_offset;
    }

    data->draw_mode = Draw_Mode::world;
    wall_draw();
    enemy_draw();
    player_draw(player);
    
    data->draw_mode = Draw_Mode::gui;
  //immediate mode ui
    Vec2 button_pos = {10, 40};
    Vec2 button_size = {16, 6};
    char buffer[256];
    sprintf_s(buffer, "selected: %s", global_ent_names[(int32)global_editor_entity_to_spawn]);
    draw_text(buffer, {button_pos.x, button_pos.y - button_size.y}, {0.5f, 0.5f});
    for (int ent_index = 0; ent_index < (int32)Ent_Type::num; ++ent_index)
    {
        Vec2 final_pos = {button_pos.x, button_pos.y + ((button_size.y + 1) * ent_index)};
        if (im_button(final_pos, button_size, ent_index))
        {
            global_editor_entity_to_spawn = (Ent_Type)ent_index;
        }
        draw_text(global_ent_names[ent_index], final_pos, {0.5, 0.5});
    }

  //debug
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
    draw_rect(input.mouse_pos_gui + Vec2{1, 1}, {2, 2}, MAGENTA);
    draw_rect(input.mouse_pos_gui, {1, 1}, LIME);

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
// draw_text_old(const char* text, Vec2 pos, Vec2 scale = {1, 1}, Vec2 spacing = {5, 8})
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
