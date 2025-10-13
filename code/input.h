#pragma once


enum struct Game_Input_Device
{
    Keyboard_Mouse,
    Controller
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
    Game_Input_Device game_input_device;
    
    Vec2f l_axes;
    Vec2f r_axes;
    
    Vec2f mouse_pos_gui;
    int32 mouse_scroll;
    
    union {
        Game_Input_Button buttons[37 + 1];
        struct {
            //GAME
            Game_Input_Button up;
            Game_Input_Button down;
            Game_Input_Button left;
            Game_Input_Button right;
            Game_Input_Button jump;
            Game_Input_Button shoot;

            //editor
            Game_Input_Button editor_toggle;
            Game_Input_Button editor_save_level;
            Game_Input_Button editor_load_level;
            
            //DEBUG
            Game_Input_Button reset;
            Game_Input_Button debug_mode_toggle;
            Game_Input_Button debug_bgmode_toggle;
            Game_Input_Button debug_win_plus;
            Game_Input_Button debug_win_minus;

            Game_Input_Button debug_hotkey1;
            Game_Input_Button debug_hotkey2;
            Game_Input_Button debug_hotkey3;

            //misc            
            Game_Input_Button shift;
            Game_Input_Button ctrl;
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
