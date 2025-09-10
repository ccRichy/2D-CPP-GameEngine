#pragma once

#include "input.h" //TODO: turn to cpp
struct Game_Data_Pointers;
//#include "game.h"

//TODO: MOVE THIS OUT LATER
bool32 collide(Vec2 pos1, Vec2 size1, Vec2 pos2, Vec2 size2, Vec2 pos_offset = {0, 0});
bool32 collide_wall(Game_Data_Pointers* game_data, Vec2 pos, Vec2 size, Vec2 pos_offset = {0, 0});


struct Player
{
    Vec2     pos;
    Vec2     spd;
    Vec2     size;
    My_Color color;

    Player* Create(Vec2 _pos, Vec2 _size, My_Color _color);
    void Update(Game_Input_Map INP);
    void Draw(Game_Data_Pointers* game_data);
};
