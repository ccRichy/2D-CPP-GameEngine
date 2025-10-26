/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */
#pragma once



enum struct Game_Input_Device
{
    Keyboard_Mouse,
    Controller
};

enum struct Input_Mode
{
    Play,
    Type
};




#define TYPING_BUFFER_SIZE 64
struct Typing_Buffer
{
    char items[TYPING_BUFFER_SIZE];
    int32 max_length;
    int32 length;
};



struct Game_Input_Button
{
    bool32 press;
    bool32 hold;
    bool32 release;

    //TODO: remove this? the ambiguity upon calling has lead to confusion
    explicit operator bool() const noexcept {
        return press;
    }
};

struct Game_Input_Map
{
    Game_Input_Device input_device;
    Input_Mode input_mode;
    
    Vec2f l_axes;
    Vec2f r_axes;
    
    Vec2f mouse_pos_gui;
    int32 mouse_scroll;
    
    union {
        Game_Input_Button buttons[40 + 1];
        struct {
            //gameplay
            Game_Input_Button up;
            Game_Input_Button down;
            Game_Input_Button left;
            Game_Input_Button right;
            Game_Input_Button jump;
            Game_Input_Button shoot;

            //editor
            Game_Input_Button edit_toggle;
            Game_Input_Button edit_level_save;
            Game_Input_Button edit_level_load;
            Game_Input_Button edit_delete;
            
            //debug
            Game_Input_Button reset;
            Game_Input_Button debug_mode_toggle;
            Game_Input_Button debug_bgmode_toggle;
            Game_Input_Button debug_win_plus;
            Game_Input_Button debug_win_minus;
            Game_Input_Button console;

            Game_Input_Button debug_hotkey1;
            Game_Input_Button debug_hotkey2;
            Game_Input_Button debug_hotkey3;

            //misc           
            Game_Input_Button ctrl;
            Game_Input_Button alt;
            Game_Input_Button shift;
            Game_Input_Button space;
            Game_Input_Button enter;
            Game_Input_Button escape;

            Game_Input_Button mouse_left;
            Game_Input_Button mouse_right;
            Game_Input_Button mouse_middle;
            Game_Input_Button mouse_back;
            Game_Input_Button mouse_front;

            Game_Input_Button num0;
            Game_Input_Button num1;
            Game_Input_Button num2;
            Game_Input_Button num3;
            Game_Input_Button num4;
            Game_Input_Button num5;
            Game_Input_Button num6;
            Game_Input_Button num7;
            Game_Input_Button num8;
            Game_Input_Button num9;            
            
            //REQUIRED: bottom of this struct for assert check
            Game_Input_Button bottom_button;
        };
    };
};


void
input_map_clear_all(Game_Input_Map* map)
{
    map->l_axes = {};
    map->r_axes = {};
    map->mouse_scroll = 0;

    i32 length = array_length(map->buttons);
    for (int i = 0; i < length; ++i)
    {
        map->buttons[i] = {};
    }
}

void
input_set_mode(Game_Input_Map* map, Input_Mode mode)
{
    map->input_mode = mode;
    input_map_clear_all(map);
} 
