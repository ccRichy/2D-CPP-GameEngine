//TODO: factor out c library
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
void debug_message(const char* fmt, ...)
{
    Debug_Message msg = {};
    int32 lifetime = DEBUG_MESSAGE_LIFETIME_DEFAULT;
    msg.lifetime = lifetime;

    char text_buffer[BUFF_LEN];
    va_list args;
    va_start(args, fmt);
    vsprintf(text_buffer, fmt, args);
    va_end(args);

    string_append(msg.text, text_buffer);
    
    pointers->data->debug_msg.current_message = msg;
    pointers->data->debug_msg.is_active = true;
}
void debug_message_queue_update()
{
    Debug_Message_Queue* queue = &pointers->data->debug_msg;
    if (queue->is_active)
    {
        Debug_Message* msg = &queue->current_message;
        if (msg->lifetime-- <= 0)
        {
            queue->is_active = false;
        }

        draw_text(msg->text, queue->pos, queue->scale);
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
inline Vec2f
mouse_get_pos_world_tile()
{
    Vec2f result = {};
    Vec2f mouse_pos_world = mouse_get_pos_world();
    result = {
        (f32)(i32)(mouse_pos_world.x / TILE_SIZE) * TILE_SIZE,
        (f32)(i32)(mouse_pos_world.y / TILE_SIZE) * TILE_SIZE,
    };
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
bool32
im_hot()
{
    b32 result = (global_imgui_hot > -1);;
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
        if (!ent->is_alive) continue;
        
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
    
    int32 alpha = 150;
    
    Color color = GREEN;
    color.a = (u8)alpha;
    draw_rect(ent_hl->pos, ent_hl->size, color);
}


void
editor_save_scene(const char* filename)
{
    char buffer[2048] = {};

    
    //save tilemap
    string_append(buffer, "Tiles\n");
    Tilemap* tmap = &pointers->data->tilemap;
    int32* grid = (int32*)&tmap->grid;
    for (int Y = 0; Y < tmap->grid_h; ++Y)
    {
        for (int X = 0; X < tmap->grid_w; ++X)
        {
            char tile_string[64];
            int32* tile = &tmap->grid[Y][X];
            if (*tile)
            {
                sprintf(tile_string, "t%i x%i y%i, ",
                        *tile, X, Y
                );

                //append
                string_append(buffer, tile_string);
                char newl[] = "\n";
            }
        }
    }

    string_append(buffer, "\n!");
    
    //save entities
    Entity* array = pointers->entity->array;
    for (int entity_index = 0; entity_index < ENT_MAX; ++entity_index)
    {
        Entity* entity = &array[entity_index];
        if (entity->is_alive){
            char ent_string[64];
            sprintf(ent_string, "e%i x%i y%i w%i h%i \n",
                    (int32)entity->type,
                    (int32)entity->pos.x,
                    (int32)entity->pos.y,
                    (int32)entity->size.x,
                    (int32)entity->size.y
            );
            
            //append
            string_append(buffer, ent_string);
        }
    }

    string_append(buffer, "!");
    
    pointers->memory->DEBUG_platform_file_write_entire(filename, string_length(buffer), &buffer);
    debug_message("Level Saved: \"%s\"", filename);
}
void editor_load_scene(const char* filename)
{
    debug_message("Level Loaded: \"%s\"", filename);
    
    entity_clear_all();    
    player_create({40, 40}); //HACK:
    
    //load file
    DEBUG_File file = pointers->memory->DEBUG_platform_file_read_entire(filename);
    char* file_mem = (char*)file.memory;

    b32 is_loading_tiles = true;
    b32 is_loading_entities = false;
    
    i32 tile_value = {};
    V2i tile_pos = {};
    
    Ent_Type type = Ent_Type::Null;
    V2f pos = {};
    V2f size = {};
    char value_string[64] = {};

    u32 beginning_of_statement_index = 0;
    u32 end_of_statement_index = 0;
    
    
    char word_buffer[64] = {};
    i32 file_mem_offset = 0;
    while ( !string_contains(word_buffer, '!') )
    {
        i32 str_len = string_get_until_space(word_buffer, file_mem + file_mem_offset);

        if (file_mem[file_mem_offset] == 't')
            tile_value = string_get_i32(word_buffer);
        if (file_mem[file_mem_offset] == 'x')
            tile_pos.x = string_get_i32(word_buffer);
        if (file_mem[file_mem_offset] == 'y')
            tile_pos.y = string_get_i32(word_buffer);

        file_mem_offset += str_len+1;
        
        if (string_contains(word_buffer, ',')) //end of single tile read
        {
            pointers->data->tilemap.grid[tile_pos.y][tile_pos.x] = tile_value;
        }
    }
    // for (uint32 char_index = 0; char_index < LOOP_MAX; ++char_index)
    // {
    //     char c = file_mem[char_index];

    //     if (c == ' ') //end of statement
    //     {
    //         end_of_statement_index = char_index;
            
    //         //get value
    //         u32 value_length = end_of_statement_index - beginning_of_statement_index;
    //         for (u32 statement_index = 0;
    //              statement_index < value_length;
    //              ++statement_index)
    //         {
    //             char cs = file_mem[statement_index + beginning_of_statement_index];
    //             value_string[statement_index] = cs;
    //         }
    //         value_string[value_length] = 0; //null terminate string 
    //         i32 value = string_get_i32(value_string);
            
    //         //get which value it is
    //         auto var = file_mem[beginning_of_statement_index];
    //         switch (var)
    //         {
    //             case 'e': type   = (Ent_Type)value; break;
    //             case 'x': pos.x  = (f32)value; break;
    //             case 'y': pos.y  = (f32)value; break;
    //             case 'w': size.x = (f32)value; break;
    //             case 'h': size.y = (f32)value; break;
    //         }            
            
    //         beginning_of_statement_index = char_index+1;

    //     }        
    //     else if (c == '\n') //end of line
    //     {
    //         Entity* entity_spawned = entity_spawn(type, pos);
    //         entity_spawned->size = size; //HACK:
    //     }
    //     else if (c == '!') //end of file
    //     {
    //         break;
    //     }
    // }
    //post-loop
}





inline int32
tilemap_get_grid_x(Tilemap* tmap, float32 x)
{
    x -= tmap->pos.x;
    int32 result = floor_i32(x / tmap->tile_w);
    return result;
}
inline int32
tilemap_get_grid_y(Tilemap* tmap, float32 y)
{
    y -= tmap->pos.y;
    int32 result = floor_i32(y / tmap->tile_h);
    return result;
}
inline Vec2i
tilemap_get_grid_pos(Tilemap* tmap, Vec2f pos)
{
    pos -= tmap->pos;
    V2i result = {
        floor_i32(pos.x / tmap->tile_w),
        floor_i32(pos.y / tmap->tile_h)
    };
    return result;
}

inline bool32
tilemap_grid_pos_within_bounds(Tilemap* tmap, Vec2i grid_pos)
{
    bool32 result = (grid_pos.y >= 0 &&
                     grid_pos.y < TILEMAP_H &&
                     grid_pos.x >= 0 &&
                     grid_pos.x < TILEMAP_W);
    return result;
}
inline bool32
tilemap_world_pos_within_bounds(Tilemap* tmap, Vec2f world_pos)
{
    V2i grid_pos = tilemap_get_grid_pos(tmap, world_pos);
    bool32 result = tilemap_grid_pos_within_bounds(tmap, grid_pos);
    return result;
}

inline int32*
tilemap_get_tile_at_world_pos(Tilemap* tmap, Vec2f pos)
{
    int32* result = 0;
    V2i tile_pos = tilemap_get_grid_pos(tmap, pos);
    result = &tmap->grid[tile_pos.y][tile_pos.x];
    return result;
}
inline void
tilemap_set_tile_at_world_pos(Tilemap* tmap, Vec2f pos, int32 value)
{
    if (tilemap_world_pos_within_bounds(tmap, pos))
    {
        int32* tile = tilemap_get_tile_at_world_pos(tmap, pos);
        *tile = value;
    }
}

void
draw_tilemap(Tilemap* tmap)
{
    //TILEMAP TEST
    int32 tile_amt = tmap->grid_w * tmap->grid_h;
    for (i32 tile_index = 0; tile_index < tile_amt; ++tile_index)
    {
        i32 tile_x = tile_index % tmap->grid_w;
        i32 tile_y = (int32)tile_index / tmap->grid_h;
        i32 tile = tmap->grid[tile_y][tile_x];
        
        if (tile == 1)
        {
            V2f final_pos = tmap->pos + V2f{(float32)tile_x * tmap->tile_w, (float32)tile_y * tmap->tile_h};
            draw_rect(
                final_pos,
                {TILE_SIZE, TILE_SIZE},
                BEIGE
            );
        }
    }    
}

bool32
collide_rect_tilemap(Vec2f pos, Vec2f size, Tilemap* tmap)
{
    bool32 result = false;

    float32 bbox_top = pos.y;
    float32 bbox_bottom = bbox_top + size.y;
    float32 bbox_left = pos.x;
    float32 bbox_right = bbox_left + size.x;
        
    int32 grid_pos_top = tilemap_get_grid_y(tmap, bbox_top);
    int32 grid_pos_bottom = tilemap_get_grid_y(tmap, bbox_bottom);
    int32 grid_pos_left = tilemap_get_grid_x(tmap, bbox_left);
    int32 grid_pos_right = tilemap_get_grid_x(tmap, bbox_right);
    
    if (grid_pos_top < 0) grid_pos_top = 0;
    if (grid_pos_bottom >= tmap->grid_h) grid_pos_bottom = tmap->grid_h - 1;
    if (grid_pos_left < 0) grid_pos_left = 0;
    if (grid_pos_right >= tmap->grid_w) grid_pos_right = tmap->grid_w - 1;
    
    for (int Y = grid_pos_top; Y <= grid_pos_bottom; ++Y)
    {
        for (int X = grid_pos_left; X <= grid_pos_right; ++X)
        {
            int32* tile = &tmap->grid[Y][X];
            if (*tile){
                V2f grid_pos_pos = tmap->pos + V2f{(f32)X * TILE_SIZE, (f32)Y * TILE_SIZE};
                if (collide_rects(pos, size, grid_pos_pos, {8, 8})){
                    result = true;
                    break;
                }
            }
        }
    }
    return result;
}
Vec2i 
collide_rect_tilemap_pos(Vec2f pos, Vec2f size, Tilemap* tmap)
{
    Vec2i result = {-1, -1};

    f32 bbox_top = pos.y;
    f32 bbox_bottom = bbox_top + size.y;
    f32 bbox_left = pos.x;
    f32 bbox_right = bbox_left + size.x;
        
    i32 grid_pos_top = tilemap_get_grid_y(tmap, bbox_top);
    i32 grid_pos_bottom = tilemap_get_grid_y(tmap, bbox_bottom);
    i32 grid_pos_left = tilemap_get_grid_x(tmap, bbox_left);
    i32 grid_pos_right = tilemap_get_grid_x(tmap, bbox_right);

    if (grid_pos_top < 0) grid_pos_top = 0;
    if (grid_pos_bottom >= tmap->grid_h) grid_pos_bottom = tmap->grid_h - 1;
    if (grid_pos_left < 0) grid_pos_left = 0;
    if (grid_pos_right >= tmap->grid_w) grid_pos_right = tmap->grid_w - 1;
    
    //HACK: maybe theres a better way to handle this?
        for (i32 Y = grid_pos_top; Y <= grid_pos_bottom; ++Y)
        {
            for (i32 X = grid_pos_left; X <= grid_pos_right; ++X)
            {
                i32* tile = &tmap->grid[Y][X];
                if (*tile){
                    V2f grid_pos_pos = tmap->pos + V2f{(f32)X * TILE_SIZE, (f32)Y * TILE_SIZE};
                    if (collide_rects(pos, size, grid_pos_pos, {8, 8}))
                    {
                        result = {X, Y};
                        break;
                    }
                }
            }
        }
    return result;
}

//TILE
Collide_Data
move_collide_tile(Tilemap* tmap, Vec2f* pos, Vec2f* spd, Vec2f size)
{
    Collide_Data coll_data = {};

    f32 caller_bbox_top;
    f32 caller_bbox_bottom;
    f32 caller_bbox_left;
    f32 caller_bbox_right;

    f32 wall_bbox_top;
    f32 wall_bbox_bottom;
    f32 wall_bbox_left;
    f32 wall_bbox_right;

    //
	pos->y += spd->y;
    caller_bbox_top = pos->y;
    caller_bbox_bottom = caller_bbox_top + size.y;
    //Entity* wall = collide_rect_get_entity(*pos, size, pointers->entity->walls, WALL_MAX);
    V2i tile_pos = collide_rect_tilemap_pos(*pos, size, tmap);
    if (tile_pos.x > -1)
    {
        wall_bbox_top = tmap->pos.y + (tile_pos.y * TILE_SIZE);
        wall_bbox_bottom = wall_bbox_top + TILE_SIZE;

        f32 y_res = pos->y;
        if (spd->y > 0)
            y_res = MIN(y_res, wall_bbox_top - size.y);
        if (spd->y < 0)
            y_res = MAX(y_res, wall_bbox_bottom);

        coll_data.vert = true;
        coll_data.ydir = sign(spd->y);
        pos->y = y_res;
        spd->y = 0;
    }

    //
	pos->x += spd->x;
    caller_bbox_left = pos->x;
    caller_bbox_right = caller_bbox_left + size.x;
    /* wall = collide_rect_get_entity(*pos, size, pointers->entity->walls, WALL_MAX); */
    tile_pos = collide_rect_tilemap_pos(*pos, size, tmap);
    if (tile_pos.x > -1)
    {
        wall_bbox_left = tmap->pos.x + (tile_pos.x * TILE_SIZE);
        wall_bbox_right = wall_bbox_left + TILE_SIZE;

        f32 x_res = pos->x;
        if (spd->x > 0)
            x_res = MIN(x_res, wall_bbox_left - size.x);
        if (spd->x < 0)
            x_res = MAX(x_res, wall_bbox_right);

        coll_data.hori = true;
        coll_data.xdir = sign(spd->x);
        pos->x = round_f32(x_res);
        spd->x = 0;
    }
    
    return coll_data;
}





extern "C" GAME_INPUT_CHANGE_DEVICE(game_input_change_device)
{ //TODO: this should most likely be platform code
    *input_map = {};
    input_map->game_input_device = input_device_current;
}




void
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

    data->state = GAME_STATE_DEFAULT;
    data->draw_mode = GAME_DRAW_MODE_DEFAULT;
    data->editor_mode = GAME_EDITOR_MODE_DEFAULT;

    data->debug_msg.pos = DEBUG_MESSAGE_POS_DEFAULT;
    data->debug_msg.scale = DEBUG_MESSAGE_SCALE_DEFAULT;
    
    data->tilemap.tile_w = TILE_SIZE;
    data->tilemap.tile_h = TILE_SIZE;
    data->tilemap.grid_w = TILEMAP_W;
    data->tilemap.grid_h = TILEMAP_H;
    data->tilemap.pos = {};
    
    data->camera_yoffset_extra = 4.f;
    data->camera_pos_offset_default = {
        BASE_W/2,
        BASE_H/2 + pointers->data->camera_yoffset_extra};
    data->camera_pos_offset = pointers->data->camera_pos_offset_default;

  //IMAGES
    //player
#define PLR_SPR_ORIGIN {6.f, 5.f}
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

    //meta
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
    // wall_create({Tile(3), Tile(20)}, {Tile(18), Tile(1)});
    // wall_create({Tile(17), Tile(20)}, {Tile(50), Tile(1)});
    // wall_create({Tile(4), Tile(11)}, {Tile(18), Tile(1)});
    // wall_create({Tile(2), Tile(11)}, {Tile(1), Tile(10)});
    // wall_create({Tile(32), Tile(11)}, {Tile(1), Tile(10)});
}




extern "C" GAME_UPDATE_AND_DRAW(game_update_and_draw)
{
    pointers = __game_pointers;

    if (!pointers->memory->is_initalized){
        //NOTE: if we assume all default init data is handled by game_initialize,
        //      we can simply zero out the necessary data here,
        //      (or why not inside the function itself), and reset is_initialized to 0
        game_initialize(pointers);
    }

    Game_Settings* settings = pointers->settings;
    Game_Data*     data     = pointers->data;
    Game_Sprites*  sprite   = &data->sprites;
    Game_Entities* entity   = &data->entity;
    Player*        player   = &entity->player;
    Tilemap* tmap = &data->tilemap;

    
    //UPDATE (early)
    camera_zoom();
    if (input.reset)
        player->pos = {BASE_W/2, 0};
    if (input.debug_mode_toggle)
        data->debug_mode_enabled = !data->debug_mode_enabled;
    

    
    //STATE UPDATE
    if (data->state == Game_State::Edit)
    {
        if (input.edit_toggle){
            data->state = Game_State::Play;
            debug_message("Game State: Play");
        }

        if (input.ctrl.hold && input.edit_level_save)
            editor_save_scene("testy_too.lvl");
        if (input.ctrl.hold && input.edit_level_load)
            editor_load_scene("testy_too.lvl");


        if (data->editor_mode == Editor_Mode::Entity)
        {
            //DEBUG Entity Editor Functions
            //select entity
            if (!im_hot())
            {
                if (input.mouse_left)
                    global_editor_selected_entity = editor_select_entity();
                //spawn entity
                if (input.mouse_right.release)
                {
                    Vec2f mouse_pos_world = mouse_get_pos_world();
                    Vec2f pos_spawn = {round_f32(mouse_pos_world.x), round_f32(mouse_pos_world.y)};
                    if (input.shift.hold)
                        pos_spawn = mouse_get_pos_world_tile();
            
                    entity_spawn(global_editor_entity_to_spawn, pos_spawn);
                }
            }
            //delete entity from room
            if (input.edit_delete){
                entity_destroy(global_editor_selected_entity);
                global_editor_selected_entity = nullptr;
            }            
        }
        else if (data->editor_mode == Editor_Mode::Tile)
        {
            if (!im_hot())
            {
                //set tiles with mouse
                if (input.mouse_left.hold){
                    tilemap_set_tile_at_world_pos(tmap, mouse_get_pos_world(), 1);
                }
                if (input.mouse_right.hold){
                    tilemap_set_tile_at_world_pos(tmap, mouse_get_pos_world(), 0);
                }
            }
            
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
        if (input.edit_toggle){
            data->state = Game_State::Edit;
            debug_message("Game State: Editor");
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


    
    //AGNOSTIC RENDERING
    data->draw_mode = Draw_Mode::World;
    {
        draw_tilemap(tmap);
        
        wall_draw();
        enemy_draw();
        player_draw(player);
        
        editor_draw_selected();
    }
    
    data->draw_mode = Draw_Mode::Gui;
    {
        im_begin();
        //draw editor_mode buttons
        Vec2f eb_pos = {40, 10};
        Vec2f eb_size = {16, 6};
        draw_text_buffer(
            {eb_pos.x, eb_pos.y - eb_size.y}, {0.5f, 0.5f}, {5, 8},
            "editor mode: %i", (i32)data->editor_mode
        );

        for (int edit_index = 0; edit_index < (int32)Editor_Mode::Num; ++edit_index)
        {
            Vec2f final_pos = {eb_pos.x + (eb_size.x + 2) * edit_index, eb_pos.y};
            if (im_button(final_pos, eb_size))
                data->editor_mode = (Editor_Mode)edit_index;
            draw_text("mode", final_pos, {0.5, 0.5});
        }
        
        
        if (data->editor_mode == Editor_Mode::Entity)
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
        else if (data->editor_mode == Editor_Mode::Tile)
        {
            
        }
    

        //debug
        debug_message_queue_update();
        IF_DEBUG
        {
            //draw performance
            auto perf = pointers->performance;
            draw_text_buffer({0, 0}, {0.5f, 0.5f}, {7, 8},
                             "ms/f: %.2f" "\n"
                             "mc/f: %.2f" "\n"
                             ,perf->ms_frame
                             ,perf->megacycles_frame);
        }

        //draw mouse
        draw_sprite_frame(&sprite->sMouse_cursors, input.mouse_pos_gui, 0);

        // //tile collision test
        // V2f m_pos = mouse_get_pos_world();
        // V2f m_scale = {8, 8};
        // Color col = GREEN;
        // if (collide_rect_tilemap(m_pos, m_scale, tmap)) col = RED;
        // draw_rect(mouse_get_pos_gui(), m_scale, col);


        // //old cursor
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
