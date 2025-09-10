#pragma once

enum struct Game_Input_Device
{
    keyboard_mouse,
    controller
};

struct Game_Input_Button
{
    bool32 press;
    bool32 hold;
    bool32 release;
};

struct Game_Input_Map
{
    Game_Input_Device game_input_device;
    
    Vec2 l_axes;
    Vec2 r_axes;
    
    union {
        Game_Input_Button buttons[16 + 1];
        struct {
            Game_Input_Button up;
            Game_Input_Button down;
            Game_Input_Button left;
            Game_Input_Button right;
            
            Game_Input_Button debug_toggle;
            Game_Input_Button debug_hotkey1;
            Game_Input_Button debug_hotkey2;
            Game_Input_Button debug_hotkey3;
            Game_Input_Button debug_hotkey4;

            Game_Input_Button debug_win_plus;
            Game_Input_Button debug_win_minus;

            Game_Input_Button shift;
            Game_Input_Button ctrl;
            Game_Input_Button space;
            Game_Input_Button enter;
            Game_Input_Button escape;

            //REQUIRED: bottom of this struct for assert check
            Game_Input_Button bottom_button;
        };
    };
};
