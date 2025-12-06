/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */

//TODO: factor out c library
//includes here will compile for only the app
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"

#include "my_color.cpp"
#include "render.cpp"
#include "collide.cpp"
#include "tilemap.cpp"
#include "player.cpp"
#include "entity.cpp"
#include "text.cpp"



//TODO: move to debug file?
void debug_message(const char* fmt, ...)
{
    Debug_Message msg = {}; //NOTE: maybe overwrite stored debug_msg directly instead
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
    if (queue->is_active){
        Debug_Message* msg = &queue->current_message;
        if (msg->lifetime-- <= 0){
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
    Vec2f result = {pointers->input->mouse_pos_gui.x / GSETTING->zoom_scale,
                    pointers->input->mouse_pos_gui.y / GSETTING->zoom_scale};
    result += GDATA->camera_pos;
    return result;
}
inline Vec2f
mouse_get_tile_pos_in_world(Tilemap* tmap) //TODO: handle tilemap pos offset!!!
{
    Vec2f result = {};
    //TODO: Tilemap* tmap = &pointers->data->tilemap;
    Vec2f mouse_pos_world = mouse_get_pos_world();
    result = {
        (f32)(i32)(mouse_pos_world.x / tmap->tile_w) * tmap->tile_w,
        (f32)(i32)(mouse_pos_world.y / tmap->tile_h) * tmap->tile_h,
    };
    return result;
}



void
camera_zoom()
{
    auto input = pointers->input;

    if (input->mouse_scroll != 0 || input->mouse_front.press)
    {
        V2f mouse_prev = mouse_get_pos_world();

        f32 zoom_amt = input->shift.hold ? 4.f : 2.f;
        if (input->mouse_front.press)
            GSETTING->zoom_scale = 1;
        else if (input->mouse_scroll > 0)
            GSETTING->zoom_scale *= (sqrt_f32(sqrt_f32(zoom_amt)));
        else if (input->mouse_scroll < 0)
            GSETTING->zoom_scale /= (sqrt_f32(sqrt_f32(zoom_amt)));
        
        V2f mouse_new = mouse_get_pos_world();
        V2f camera_pos_current = GDATA->camera_pos;
        GDATA->camera_pos =
            {camera_pos_current.x + mouse_prev.x - mouse_new.x,
             camera_pos_current.y + mouse_prev.y - mouse_new.y};

        debug_message("Zoom: %f", GSETTING->zoom_scale);
    }
}



////////UI STUFF/////////
//im_gui
globalvar int32 global_imgui_hot = -1;
globalvar int32 global_imgui_active = -1;
globalvar int32 global_imgui_num = 0;

void
im_begin()
{
    global_imgui_hot = -1;
    global_imgui_num = 0;
}
bool32
im_button(Vec2f button_pos, Vec2f button_size)//, int32 id)
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
im_button_text(Vec2f button_pos, Vec2f button_size, const char* text, V2f text_offset = {})
{
    b32 result = im_button(button_pos, button_size);
    draw_text(text, button_pos + text_offset);
    return result;
}

bool32
im_hot()
{
    b32 result = (global_imgui_hot > -1);
    return result;
}




//Editor
globalvar bool32 global_editor_level_changed; //used to keep level changes without commiting them to disk
globalvar Ent_Type global_editor_entity_to_spawn = Ent_Type::Player;
globalvar Entity* global_editor_selected_entity = nullptr;


Entity*
entity_spawn(Ent_Type type, Vec2f pos)
{
    using enum Ent_Type;
    Entity* result = nullptr;

    switch (type){
      case Player: PLAYER->create(pos); break;
      case Wall:  result  = wall_create(pos, {Tile(1), Tile(1)}); break;
      default: entity_init(type, pos);
    }

    return result;
}

Entity*
entity_at_mouse_pos()
{
    Entity* result = collide_pixel_entity_pointer(mouse_get_pos_world(), Ent_Type::All);
    return result;
}
void
editor_draw_selected()
{
    Entity* ent_hl = global_editor_selected_entity;
    if (!ent_hl) return;
    GDATA->draw_mode = Draw_Mode::World;
    
    int32 alpha = 150;
    
    Color color = GREEN;
    color.a = (u8)alpha;
    draw_rect(ent_hl->pos, ent_hl->bbox.size, color);
}

bool32
is_level_new(char* level_name)
{
    b32 result = (level_name[0] == 0 || string_equals(level_name, UNSAVED_BACKUP_NAME));
    return result;
}
void 
level_clear()
{
    entity_clear_all();
    tilemap_clear_all(&pointers->data->tilemap);
    PLAYER->create({BASE_W/2, BASE_H/2});
    string_clear(pointers->data->level_current);
};
bool32
level_save_file(const char* filename)
{
    if (filename[0] == 0){
        debug_message("Level failed to save: empty string");
        return false;
    }
    
    //HACK:? temporaryily allocate storage for save file
    //TODO: make functions for opening, appeding, and closing a file
    int32 buffer_size_bytes = Megabytes(SAVE_FILE_BUFFER_MB);
    char* buffer = new char[buffer_size_bytes](); // allocated on the heap
    
//player
    Player* player = PLAYER;
    string_append(buffer, "Player\n");
    char player_string[64];
    sprintf(player_string, "p x%i y%i\n",
           (i32)player->pos.x, (i32)player->pos.y
    );
    string_append(buffer, player_string);
    string_append(buffer, "!\n\n");    
    
//tilemap
    string_append(buffer, "Tiles\n");
    Tilemap* tmap = &pointers->data->tilemap;
    Tile* grid = (Tile*)&tmap->grid;
    for (int Y = 0; Y < tmap->grid_h; ++Y){
        for (int X = 0; X < tmap->grid_w; ++X){
            char tile_string[64];
            Tile* tile = &tmap->grid[Y][X];
            if (*tile){
                sprintf(tile_string, "t%i x%i y%i, ",
                        *tile, X, Y
                );
                string_append(buffer, tile_string);
            }
        }
    }
    string_append(buffer, "\n!\n\n");
    
//entities
    string_append(buffer, "Entities\n");
    Entity* array = pointers->entity->array;
    for (int entity_index = 0; entity_index < ENT_MAX_ALL(); ++entity_index){
        Entity* entity = &array[entity_index];
        if (entity->is_alive){
            char ent_string[64];
            sprintf(ent_string, "e%i x%i y%i w%i h%i,\n",
                    (int32)entity->type,
                    (int32)entity->pos.x,
                    (int32)entity->pos.y,
                    (int32)entity->bbox.size.x,
                    (int32)entity->bbox.size.y
            );
            string_append(buffer, ent_string);
        }
    }
    string_append(buffer, "!");

    //construct filepath
    char file_path[256] = "levels\\";
    string_append(file_path, filename);
    string_append(file_path, ".lvl");

    //save
    int32 buffer_length = string_length(buffer) * sizeof(char);
    pointers->memory->DEBUG_platform_file_write_entire(file_path, buffer_length, (char*)buffer);

    //set current level
    if (!string_equals(GDATA->level_current, filename)){
        string_clear(GDATA->level_current);
        string_append(GDATA->level_current, filename);
    }

    //log
    debug_message("Level Saved: \"%s\"", file_path);
    free(buffer);
    return true;
}
bool32
level_load_file(const char* filename) //filename without extension
{
    //TODO: REQUIRED: 
    // Currently to get the string for the specific type of data, we use string_get_delimiter() on the file's allocated memory.
    // The problem is we treat the raw file memory as the string AND the buffer, so the function appends 0 to it to null-terminate. 
    // The layout of our strings *just so happens* to allow this to work anyway.
    // Fix: dont rely on chunking out the data with string_get_delimiter()

//construct filepath
    char file_path[256] = "levels\\";
    string_append(file_path, filename);
    string_append(file_path, ".lvl");
    
    //load file
    DEBUG_File file = pointers->memory->DEBUG_platform_file_read_entire(file_path);
    if (!file.memory){
        debug_message("Level couldn't load: \"%s\" doesn't exist", file_path);
        return false;
    }
    char* file_mem = (char*)file.memory;

    //clear data
    Tilemap* tmap = &pointers->data->tilemap;
    level_clear();
    
    V2i player_pos = {};

    i32 tile_value = 0;
    V2i tile_pos = {};
    
    Ent_Type ent_type = Ent_Type::Null;
    V2f ent_pos = {};
    V2f ent_size = {};
    char value_string[64] = {};
    
    i32 buff_len = 0;
    i32 buff_offset = 0;
    i32 file_mem_offset = 0;
    char word_buffer[64] = {};

//PLAYER
    char* player_buff = file_mem;
    buff_len = string_get_until_delimiter(player_buff, file_mem, '!');
    while ( buff_offset < buff_len )
    {
        i32 word_len = string_get_until_space(word_buffer, &player_buff[buff_offset]);

        if (word_buffer[0] == 'x')
            player_pos.x = string_get_i32(word_buffer);
        else if (word_buffer[0] == 'y')
            player_pos.y = string_get_i32(word_buffer);

        buff_offset += word_len+1;
    }
    PLAYER->create(v2i_to_v2f(player_pos));
    file_mem_offset += buff_len;
    buff_offset = 0;


//TILES
    char* tiles_buff = file_mem + file_mem_offset;
    // char tiles_buff[SAVE_BUFFER_MAX] = {};
    buff_len = string_get_until_delimiter(tiles_buff, file_mem + file_mem_offset, '!');

    while ( buff_offset < buff_len )
    {
        i32 word_len = string_get_until_space(word_buffer, tiles_buff + buff_offset);

        auto var = word_buffer[0];
        i32 value = string_get_i32(word_buffer);
        switch (var){
          case 't': tile_value = value; break;
          case 'x': tile_pos.x = value; break;
          case 'y': tile_pos.y = value; break;
        }

        buff_offset += word_len+1;
        
        if (string_contains(word_buffer, ',')){ //end of single tile read
            tmap->grid[tile_pos.y][tile_pos.x] = tile_value;
        }
    }
    file_mem_offset += buff_len;
    buff_offset = 0;


//ENTITY
    char* entity_buff = file_mem + file_mem_offset;
    buff_len = string_get_until_delimiter(entity_buff, file_mem + file_mem_offset, '!');
    while ( buff_offset < buff_len )
    {
        i32 word_len = string_get_until_space(word_buffer, entity_buff + buff_offset);

        auto var = word_buffer[0];
        i32 value = string_get_i32(word_buffer);
        switch (var){
          case 'e': ent_type   = (Ent_Type)value; break;
          case 'x': ent_pos.x  = (f32)value;      break;
          case 'y': ent_pos.y  = (f32)value;      break;
          case 'w': ent_size.w = (f32)value;      break;
          case 'h': ent_size.h = (f32)value;      break;
        }            

        buff_offset += word_len+1;

        if (string_contains(word_buffer, ',')){
            Entity* entity_spawned = entity_spawn(ent_type, ent_pos);
            //HACK:
            if (ent_type == Ent_Type::Wall)
                entity_spawned->bbox.size = ent_size;
            //TODO: dont save unneccesary size data for entities
        }
    }

    //change current level string
    if (!string_equals(pointers->data->level_current, filename))
    {
        string_clear(pointers->data->level_current);
        string_append(pointers->data->level_current, filename);
    }

    debug_message("Level Loaded: \"%s\"", filename);
    GMEMORY->DEBUG_platform_file_free_memory(file.memory);
    return true;
}
void
level_reload()
{
    char current_level_name[64] = {};
    string_append(current_level_name, pointers->data->level_current);
    level_load_file(current_level_name);
}



void
game_save_state(const char* filename) //this dumps the whole ass raw memory
{
    char dirbuf[128] = "misc\\";
    string_append(dirbuf, filename);
    b32 result = false;
    if (GMEMORY->DEBUG_platform_file_write_entire)
        result = GMEMORY->DEBUG_platform_file_write_entire(dirbuf, sizeof(Game_Data), pointers->data);
    
    if (result)
        debug_message("game state saved: \"%s\"", filename);
    else
        debug_message("game state failed to save: \"%s\"", filename);
}
void
game_load_state(const char* filename) //trying to load old versions is likely to break
{
    char dirbuf[128] = "misc\\";
    string_append(dirbuf, filename);

    //load file
    DEBUG_File file = pointers->memory->DEBUG_platform_file_read_entire(dirbuf);
    if (!file.memory){
        debug_message("couldnt load game state: \"%s\" doesn't exist?", filename);
    }else{
        Game_Data* data = (Game_Data*)file.memory;
        GDATA->tilemap = data->tilemap;
        GDATA->entity = data->entity;
        debug_message("level state loaded: \"%s\"", dirbuf);
    }
    GMEMORY->DEBUG_platform_file_free_memory(file.memory);
}


void
draw_bg(Sprite* spr)
{   
    auto prev_draw_mode = GDATA->draw_mode;
    GDATA->draw_mode = Draw_Mode::Gui;
    draw_sprite(spr, {});
    GDATA->draw_mode = prev_draw_mode;
}

//important stuff
void
game_draw_world()
{
    GDATA->draw_mode = Draw_Mode::Gui;
    draw_bg(&GSPRITE->sBG_cave1);

    GDATA->draw_mode = Draw_Mode::World;
    // for (int i = 0; i < 10; ++i){
    //     draw_rect({ (f32)i * 16, 0}, {8, 8}, RED);
    // }

    draw_tilemap(&GDATA->tilemap);

    // wall_draw();
    spike_draw();
    // enemy_draw();
    PLAYER->draw();
}


void
game_state_editor_update()
{
    auto input = GINPUT;
    auto data = GDATA;
    auto edit_mode = GDATA->editor_mode;
    auto tmap = &GDATA->tilemap;

    game_draw_world();
    
//Controls
    if (input->edit_toggle){
        if (is_level_new(GDATA->level_current))
            level_save_file(UNSAVED_BACKUP_NAME);
        
        GDATA->state = Game_State::Play;
        debug_message("Game State: Playing \"%s\"", GDATA->level_current);
    }

//save/load
    if (input->edit_save && input->ctrl.hold){
        if (is_level_new(data->level_current)){
            debug_message("Level Save: Save new level with `console: \"save [name]\"");
        }else{
            level_save_file(data->level_current);
        }
    }
    if (input->edit_load && input->ctrl.hold)
        level_reload();

    
//Edit states
    if (edit_mode == Editor_Mode::Entity)
    {
        editor_draw_selected();
        
        //DEBUG Entity Editor Functions
        if (!im_hot()){
            if (input->mouse_left){
                if (input->alt.hold){
                    //spawn
                    Vec2f spawn_pos = mouse_get_tile_pos_in_world(tmap); //round to tile
                    if (input->shift.hold)
                        spawn_pos = round_v2f(mouse_get_pos_world()); //mouse pixel
                    entity_spawn(global_editor_entity_to_spawn, spawn_pos);
                }else{
                    //select
                    global_editor_selected_entity = entity_at_mouse_pos();
                }
            }
        }
        //delete entity from room
        if (input->mouse_right.release){
            entity_destroy(global_editor_selected_entity);
            global_editor_selected_entity = nullptr;
        }            
    }
    else if (edit_mode == Editor_Mode::Tile)
    {
//selection test
        persist V2f sel_pos1 = {};
        persist b32 is_dragging_selection = false;
        persist Rect sel_rect_final = {};
        Color sel_col = WHITE;
        sel_col.a = 40;

        b32 select_input = input->ctrl.hold;
        if (input->mouse_left.press && select_input){ //start
            sel_pos1 = mouse_get_pos_world();
            is_dragging_selection = true;
        }
        if (input->mouse_left.hold && select_input){ //drag
            //TODO: maybe handle draw_rect() negative scale
            V2f sel_pos2 = mouse_get_pos_world();
            V2f sel_pos_diff = sel_pos2 - sel_pos1;
            f32 selx = sel_pos_diff.x > 0 ? sel_pos1.x : sel_pos2.x;
            f32 sely = sel_pos_diff.y > 0 ? sel_pos1.y : sel_pos2.y;
            sel_rect_final = {.pos = {selx, sely}, .size = abs_v2f(sel_pos_diff)};
        }
        if (input->mouse_left.release){ //fill grid space
            f32 sel_rect_xprev = sel_rect_final.x;
            sel_rect_final.x = floor_f32(sel_rect_final.x / tmap->tile_w) * tmap->tile_w;
            f32 sel_rect_xprev_diff = sel_rect_xprev - sel_rect_final.x;
            sel_rect_final.w += sel_rect_xprev_diff;
            sel_rect_final.w = ceil_f32(sel_rect_final.w / tmap->tile_w) * tmap->tile_w;
            
            f32 sel_rect_yprev = sel_rect_final.y;
            sel_rect_final.y = floor_f32(sel_rect_final.y / tmap->tile_h) * tmap->tile_h;
            f32 sel_rect_yprev_diff = sel_rect_yprev - sel_rect_final.y;
            sel_rect_final.h += sel_rect_yprev_diff;
            sel_rect_final.h = ceil_f32(sel_rect_final.h / tmap->tile_h) * tmap->tile_h;
            is_dragging_selection = false;
        }
        if (input->mouse_right.press && select_input){ //remove
            sel_rect_final = {};
        }

        //TODO: test tilemap_get_grid_x and y overshooting 1 pixel
        if (input->edit_delete.press){ //delete tiles in selection
            int32 grid_pos_top    = tilemap_get_grid_y(tmap, sel_rect_final.y);
            int32 grid_pos_bottom = tilemap_get_grid_y(tmap, sel_rect_final.y + sel_rect_final.h) - 1;
            int32 grid_pos_left   = tilemap_get_grid_x(tmap, sel_rect_final.x);
            int32 grid_pos_right  = tilemap_get_grid_x(tmap, sel_rect_final.x + sel_rect_final.w) - 1;
            for (int Y = grid_pos_top; Y <= grid_pos_bottom; ++Y){
                for (int X = grid_pos_left; X <= grid_pos_right; ++X){
                    int32* tile = &tmap->grid[Y][X];
                    *tile = 0;
                }
            }

        }
        
        //draw selection
        draw_rect(sel_rect_final, sel_col);

        
//set tiles
        if (!is_dragging_selection && !select_input){
            if (!im_hot()){
                if (input->mouse_left.hold){
                    tilemap_set_tile_at_world_pos(tmap, mouse_get_pos_world(), 1);
                }
                if (input->mouse_right.hold){
                    tilemap_set_tile_at_world_pos(tmap, mouse_get_pos_world(), 0);
                }
            }
        }
    }

        
//Draw 0,0 (x,y) lines
    data->draw_mode = Draw_Mode::World;
    f32 static_size = scale_get_zoom_agnostic(0.5);
    draw_rect({-static_size, -static_size}, {static_size, 999999.f}, WHITE);
    draw_rect({-static_size, -static_size}, {999999.f, static_size}, WHITE);

//Draw grid //TODO: make straight line functions for this use case
    V2i   grid_length_max = {1000, 1000};
    i32   grid_spacing    = TILE_SIZE;
    f32   grid_line_size  = (1.f/pointers->settings->window_scale) * static_size;
    u8    color_val = 50;
    Color color_faded     = {color_val, color_val, color_val, 255};
    for (int grid_x = 0; grid_x < grid_length_max.x; ++grid_x){
        draw_rect(
            {(float32)grid_x * grid_spacing, 0},
            {grid_line_size, (f32)grid_length_max.x * grid_spacing},
            color_faded);
    }
    for (int grid_y = 0; grid_y < grid_length_max.y; ++grid_y){
        draw_rect(
            {0, (float32)grid_y * grid_spacing},
            {(f32)grid_length_max.x * grid_spacing, grid_line_size},
            color_faded);
    }

//Camera
    f32 cam_speed = 2;
    if (input->ctrl.hold) cam_speed = cam_speed/2;
    if (input->shift.hold) cam_speed = cam_speed*2;
        
    V2f move_input =
        {(float32)(input->right.hold - input->left.hold) / GSETTING->zoom_scale,
         (float32)(input->down.hold - input->up.hold) / GSETTING->zoom_scale};
    data->camera_pos += move_input * Vec2f{cam_speed, cam_speed};

    
//Draw GUI
    data->draw_mode = Draw_Mode::Gui;
    Vec2f eb_pos = {40, 10};
    Vec2f eb_size = {16, 6};
    draw_text_buffer(
        {eb_pos.x, eb_pos.y - eb_size.y}, {0.5f, 0.5f}, {5, 8},
        "editor mode: %i", (i32)data->editor_mode
    );

    //edit mode buttons
    im_begin();
    for (int edit_index = 0; edit_index < (int32)Editor_Mode::Num; ++edit_index){
        Vec2f final_pos = {eb_pos.x + (eb_size.x + 2) * edit_index, eb_pos.y};
        if (im_button(final_pos, eb_size))
            data->editor_mode = (Editor_Mode)edit_index;
        draw_text("mode", final_pos, {0.5, 0.5});
    }

    //entity buttons
    if (data->editor_mode == Editor_Mode::Entity)
    {
        Vec2f button_pos = {10, 40};
        Vec2f button_size = {16, 6};
        draw_text_buffer(
            {button_pos.x, button_pos.y - button_size.y}, {0.5f, 0.5f}, {5, 8},
            "selected:%s", ENT_NAME(global_editor_entity_to_spawn)
        );

        for (int ent_index = 0; ent_index < (int32)Ent_Type::Num; ++ent_index)
        {
            Vec2f final_pos = {button_pos.x, button_pos.y + ((button_size.y + 1) * ent_index)};
            if (im_button(final_pos, button_size))
                global_editor_entity_to_spawn = (Ent_Type)ent_index;
            draw_text(ENT_INFO[ent_index].name, final_pos, {0.5, 0.5});
        }
            
    }
    else if (data->editor_mode == Editor_Mode::Tile)
    {
        //TODO: tile types
    }
}



void
game_state_play_update()
{
    auto input = GINPUT;
    auto data = GDATA;
    
    //controls
    if (input->edit_toggle){
        level_reload();
        data->state = Game_State::Edit;
        debug_message("Game State: Editor");
    }
    if (input->escape.press)
        data->state = Game_State::Pause;

    

//teleport player to mouse
    IF_DEBUG {
        if (input->mouse_right.hold){
            data->draw_mode = Draw_Mode::World;
            draw_sprite_frame(&GSPRITE->sPlayer_idle, mouse_get_pos_world(), 0);
        }
        if (input->mouse_right.release){
            PLAYER->pos = mouse_get_pos_world();
        }
    }

    
    //entity update
    PLAYER->update();
    // entity_update();
    enemy_update();

    //camera update
    float32 cam_yoffset_extra = 4;
    Vec2f vec2_zoom = {GSETTING->zoom_scale, GSETTING->zoom_scale};
    data->camera_pos_offset = data->camera_pos_offset_default / vec2_zoom;
    data->camera_pos = PLAYER->pos - data->camera_pos_offset;

    data->camera_pos.x = clamp_f32(data->camera_pos.x, 0, 1000);
    data->camera_pos.y = clamp_f32(data->camera_pos.y, 0, 1000);
    
    game_draw_world();
}

void
game_state_pause_update()
{
    auto input = GINPUT;
    auto data = GDATA;

//controls
    if (input->escape.press)
        data->state = Game_State::Play;
    
//world
    game_draw_world();

//gui
    data->draw_mode = Draw_Mode::Gui;
    draw_text("PAUSED", {BASE_W/2 - 20, 20}, {4, 4});

    V2f buttpos = BASE_CENTER_V2F;
    V2f buttsize = {80, 8};
    f32 spacing = 12;
    for (int button_index = 0; button_index < 4; ++button_index)
    {
        im_button_text(buttpos + V2f{0, button_index * spacing}, buttsize, "poop", {30, 0});
    }
}






extern "C" GAME_INPUT_CHANGE_DEVICE(game_input_change_device)
{ //TODO: this should be on the platform layer
    input_map_clear_all(input_map);
    input_map->input_device = input_device_current;
}




void
game_initialize(Game_Pointers* _game_pointers)
{
    pointers->memory->is_initalized = true;

    Game_Input_Map* input    = pointers->input;
    Game_Data*      data     = pointers->data;
    Game_Entities*  entity   = &data->entity;
    Game_Sprites*   sprite   = &data->sprites;
    Player*         player   = &entity->player;
    Game_Settings*  settings = pointers->settings;

    i64 set_input_array_to_this_value = &input->bottom_button - &input->buttons[0] + 1;
    i32 button_length = array_length(input->buttons);
    if (set_input_array_to_this_value != button_length) Assert(false); //input union
    Assert( sizeof(Game_Data) <= pointers->memory->permanent_storage_space ); //memory bounds
    // data->memory_map.init(null[t]);
    
    data->state       = GSTATE_DEFAULT;
    data->editor_mode = GEDITOR_MODE_DEFAULT;
    data->draw_mode   = Draw_Mode::World;

    data->debug_msg.pos   = DEBUG_MESSAGE_POS_DEFAULT;
    data->debug_msg.scale = DEBUG_MESSAGE_SCALE_DEFAULT;

    data->tilemap.tile_w = TILE_SIZE;
    data->tilemap.tile_h = TILE_SIZE;
    data->tilemap.grid_w = TILEMAP_W;
    data->tilemap.grid_h = TILEMAP_H;
    data->tilemap.pos    = {0, 0};

    data->camera_yoffset_extra = 4.f;
    data->camera_pos_offset_default = {
        BASE_W/2,
        BASE_H/2 + pointers->data->camera_yoffset_extra};
    data->camera_pos_offset = pointers->data->camera_pos_offset_default;

    //initialize array of pointers to the 1st entity for each one
    Entity* entity_pointer = pointers->entity->array;
    for (int i = 0; i < (i32)Ent_Type::Num; ++i){
        if (i == 0){ //player
            // entity->pointers[i] = nullptr;
        }else{
            i32 current_entity_max = ENT_INFO[i].max_count;
            entity->pointers[i] = entity_pointer;

            //DEBUG: set type for validation
            for (int jj = 0; jj < current_entity_max; ++jj){
                entity_pointer[jj].type = (Ent_Type)i;
            }

            entity_pointer += current_entity_max;
        }
    }

#define XMAC(name, ...) sprite->##name = sprite_create(#name, __VA_ARGS__);
    SPR_LIST
#undef XMAC

#define BMP_LOAD(name) data->##name = DEBUG_load_bmp(#name)
    BMP_LOAD(sTest);
    BMP_LOAD(sTest_wide);
    BMP_LOAD(sMan);
    BMP_LOAD(sMan_anim);
    BMP_LOAD(sFont_test);
    BMP_LOAD(sFont_ASCII_lilliput);
    BMP_LOAD(sFont_ASCII_lilliput_vert);

    font_create(&data->sFont_ASCII_lilliput_vert, 8);
    
    level_load_file(LEVEL_FIRST);
}





extern "C" GAME_UPDATE_AND_DRAW(game_update_and_draw)
{
    //NOTE: these are here (updated every frame) so we dont have to worry about reassignment upon hot-reload
    //TODO: assign these at hot-reload/recompiling point
    pointers = __game_pointers;
    PLAYER   = pointers->player;
    GSETTING = pointers->settings;
    GINPUT   = pointers->input;
    GMEMORY  = pointers->memory;
    GDATA    = pointers->data;
    GENTITY  = &GDATA->entity;
    GSPRITE  = &GDATA->sprites;

    if (!pointers->memory->is_initalized){
        //NOTE: if we assume all default init data is handled by game_initialize,
        //      we can simply zero out the necessary data here,
        //      (or why not inside the function itself), and reset is_initialized to 0
        game_initialize(pointers);
    }

    Game_Data*     data = GDATA;
    Tilemap*       tmap      = &data->tilemap;
    Game_State     state     = data->state;
    Editor_Mode    edit_mode = data->editor_mode;
    Typing_Buffer* console    = pointers->console;
    
    
//DEBUG INPUTS
    camera_zoom();
    
    if (input->edit_save && !input->ctrl.hold)
        game_save_state("state.st");
    if (input->edit_load && !input->ctrl.hold)
        game_load_state("state.st");

    if (input->reset)
        level_reload();
        
    if (input->debug_mode_toggle){
        data->debug_mode_enabled = !data->debug_mode_enabled;\
        const char* dbm_str = data->debug_mode_enabled ? "Enabled" : "Disabled";
        debug_message("Debug Mode: %s", dbm_str);
    }


//Game State
    if (state == Game_State::Edit)       game_state_editor_update();
    else if (state == Game_State::Play)  game_state_play_update();
    else if (state == Game_State::Pause) game_state_pause_update();

    //TODO: move this into editor update
    // editor_draw_selected();

    //debug
    data->draw_mode = Draw_Mode::Gui;
    debug_message_queue_update();
    IF_DEBUG {
        //draw performance
        auto perf = pointers->performance;
        draw_text_buffer({77, 0}, {0.77f, 0.77f}, {7, 8},
                         "ms/f: %.2f" "\n"
                         "mc/f: %.2f" "\n"
                         ,perf->ms_frame
                         ,perf->megacycles_frame);
    }
    // GSETTING->zoom_scale = 0.77f;

//Console
    
    //TODO:
    //- show typing cursor
    persist char cmd_string[64];
    persist char args_string[64];
    persist i32 args_index;
    char* level_current = data->level_current;
    
    if (input->console.press)
        input_set_mode(input, Input_Mode::Type);
    
    if (input->input_mode == Input_Mode::Type)
    {
        //draw console winddow
        V2f console_pos = {50, 50};
        draw_rect(console_pos, {BASE_W, 10}, MAGENTA);
        draw_text(console->items, console_pos);

        //confirm input
        if (input->enter.press){
            //get command and argument(s) strings
            //args_index gives us the offset to the start of the args_string
            args_index = 1 + string_get_until_space(cmd_string, console->items);
            string_get_until_space(args_string, &console->items[ args_index ]);
            
            if (string_equals(cmd_string, "load")){
                level_load_file(args_string);
            }
            else if (string_equals(cmd_string, "save")){
                level_save_file(args_string);
            }
            else if (string_equals(cmd_string, "clear")){
                level_clear();
            }
            else if (string_equals(cmd_string, ">:)")){
                PLAYER->state_switch(Player_State::Hurt);
            }
            
            //reset
            for (int i = 0; i < console->length; ++i){
                console->items[i] = 0;
            }
            console->length = 0;
            input_set_mode(input, Input_Mode::Play);
        }
    }

    //draw mouse
        draw_sprite_frame(&GSPRITE->sMouse_cursors, input->mouse_pos_gui, 0);
}
