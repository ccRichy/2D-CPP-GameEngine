#pragma once

/*

  NOTE: Currently passed by value for simplicity sake.
  However, if we wanted to do things like: disable certain inputs in the middle of the game loop,
  We need to start passing this by pointer

 */

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

    explicit operator bool() const noexcept {
        return press;
    }

};

struct Game_Input_Map
{
    Game_Input_Device game_input_device;
    
    Vec2 l_axes;
    Vec2 r_axes;
    
    Vec2 mouse_pos;
    
    union {
        Game_Input_Button buttons[33 + 1];
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
            
            //DEBUG
            Game_Input_Button reset;
            Game_Input_Button debug_toggle;
            Game_Input_Button debug_bgmode;
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

            Game_Input_Button left_click;
            Game_Input_Button right_click;
            Game_Input_Button middle_click;

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
