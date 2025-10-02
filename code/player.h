#pragma once

#include "input.h"
struct Game_Pointers;
struct Sprite;



enum struct Player_State
{
    ground,
    air
};

struct Player
{
    Vec2     pos;
    Vec2     spd;
    Vec2     size;
    Vec2     scale;
    Color    color;

    //anim
    Sprite* sprite;
    float32 anim_index;
    float32 anim_speed_mult;
    
    Player_State state;
    float32 grav_default;
    float32 grav_low;
    float32 grav;
    int32 aim_dir;

    Player* Create(Vec2 _pos);
    void Update(Game_Pointers game_pointers, Game_Input_Map input);
    void Draw(Game_Pointers game_pointers);
};
