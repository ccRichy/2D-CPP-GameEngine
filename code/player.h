#pragma once

#include "input.h" //TODO: turn to cpp
struct Game_Pointers;



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
    Color    color;
    Player_State state;

    int8 aim_dir;

    Player* Create(Vec2 _pos);
    void Update(Game_Pointers game_pointers, Game_Input_Map input);
    void Draw(Game_Pointers game_pointers);
};
