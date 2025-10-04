#pragma once


struct Collide_Data
{
    bool32 hori;
    bool32 vert;
    int32 xdir;
    int32 ydir;
    Vec2 spd;
};


bool32 collide(Vec2 pos1, Vec2 size1, Vec2 pos2, Vec2 size2);
bool32 on_screen(Vec2 pos, Vec2 padding = {});

// Collide_Data move_collide_wall(Game_Pointers* game_pointers, Vec2* pos, Vec2* spd, Vec2 size)
