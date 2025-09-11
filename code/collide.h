#pragma once

struct Rectangle
{
    Vec2 pos;
    Vec2 size;
};

struct Collide_Data
{
    bool32 hori;
    bool32 vert;
    Vec2 dir;
    Vec2 spd;
};

bool32 collide(Vec2 pos1, Vec2 size1, Vec2 pos2, Vec2 size2, Vec2 pos_offset = {0, 0});
bool32 collide_wall(Game_Data_Pointers game_data, Vec2 pos, Vec2 size, Vec2 pos_offset = {0, 0});
Rectangle collide_wall_rect(Game_Data_Pointers game_data, Vec2 pos, Vec2 size, Vec2 pos_offset = {0, 0});
Collide_Data move_collide_wall(Game_Data_Pointers game_data, Vec2* pos, Vec2* spd, Vec2 size);
