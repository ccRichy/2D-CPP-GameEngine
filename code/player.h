#pragma once

// #include 
#include "my_color.h"
struct Sprite;


enum struct Player_State
{
    idle,
    move,
    jump,
    fall,
    rope
};
struct State
{
    void (*Enter)();
    void (*Step)(Game_Input_Map input);
};
struct Physics
{
    float32 accel;
    float32 decel;
    float32 turn;
    float32 grav;
};

struct Player
{
    Vec2f pos;
    Vec2f move_input;
    Vec2f spd;
    Vec2f size;
    Vec2f scale;
    Color color;

    //numbas
    Physics physics;
    Physics ground_physics;
    Physics jump_physics;
    Physics fall_physics;
    
    float32 jump_spd;
    float32 ground_speed_max;
    float32 grav_default;
    float32 grav_low;
    float32 grav;
    float32 terminal_velocity;

    int32 aim_dir;

    //anim
    Sprite* sprite;
    float32 anim_index;
    float32 anim_speed_mult;

#if 0
    Player_State state;
    State* state;
    State st_idle;
    State st_walk;
    State st_jump;
    State st_fall;
    State st_ledge;
#endif
};
