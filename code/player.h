#pragma once

#include "input.h"
struct Game_Pointers;
struct Sprite;



enum struct Player_State
{
    idle,
    move,
    jump,
    fall,
    rope
};


struct Player
{
    Vec2     pos;
    Vec2     move_input;
    Vec2     spd;
    Vec2     size;
    Vec2     scale;
    Color    color;

    //numbas
    float32 ground_speed_max;
    
    //anim
    Sprite* sprite;
    float32 anim_index;
    float32 anim_speed_mult;
    
    Player_State state;
    float32 grav_default;
    float32 grav_low;
    float32 grav;
    int32 aim_dir;
};
