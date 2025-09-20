#pragma once

#include "input.h" //TODO: turn to cpp
struct Game_Pointers;



enum struct Player_States
{
    ground,
    air
};

struct Player
{
    Vec2     pos;
    Vec2     spd;
    Vec2     size;
    Color    color;
    Player_States state;

    int8 aim_dir;
    Vec2 origin;

    Player* Create(Vec2 _pos);
    void Update(Game_Pointers game_pointers, Game_Input_Map INP);
    void Draw(Game_Pointers game_pointers);
};
