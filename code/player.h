#pragma once

#include "input.h" //TODO: turn to cpp
struct Game_Data_Pointers;



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

    Player* Create(Vec2 _pos, Vec2 _size, Color _color);
    void Update(Game_Data_Pointers game_data, Game_Input_Map INP);
    void Draw(Game_Data_Pointers game_data);
};
