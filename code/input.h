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
};

struct Game_Input_Map
{
    Game_Input_Device game_input_device;
    
    Vec2 l_axes;
    Vec2 r_axes;
    
    union {
        Game_Input_Button buttons[19 + 1];
        struct {
            //GAME
            Game_Input_Button up;
            Game_Input_Button down;
            Game_Input_Button left;
            Game_Input_Button right;
            Game_Input_Button jump;
            Game_Input_Button shoot;

            //DEBUG
            Game_Input_Button reset;
            Game_Input_Button debug_toggle;
            Game_Input_Button debug_bgmode;
            Game_Input_Button debug_win_plus;
            Game_Input_Button debug_win_minus;

            Game_Input_Button debug_hotkey1;
            Game_Input_Button debug_hotkey2;
            Game_Input_Button debug_hotkey3;


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
