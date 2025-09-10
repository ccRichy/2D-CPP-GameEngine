#pragma once

#include "my_vec.h"
#include "game.h"
#include "my_color.h"


//TODO: Maybe consider an Entity 'component' aka another struct that just includes
//      the common elements so we can work on all of that data
#define WALL_MAX 32
struct Walls
{
    int8 count;
    Vec2     pos[WALL_MAX];
    Vec2     size[WALL_MAX];
    My_Color color[WALL_MAX];
    
    Walls* Create(Vec2 _pos, Vec2 _size, My_Color _color);
    void Draw(Game_Data_Pointers* game_data);
};
