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


// #define collide_entity(return_value, entity_struct, __pos, __size, pos_offset)\
// auto entitys = &entity_struct;\
// auto __ret_val = &return_value;\
// for (int __it = 0; __it < entitys->count_alive; ++__it)\
// {\
//     Vec2 ___pos = entitys->pos[__it];\
//     Vec2 ___size = entitys->size[__it];\
//     if (collide(__pos, __size, ___pos, ___size, pos_offset))\
//         *__ret_val = true;\
// }\
// *__ret_val = false;



bool32 collide(Vec2 pos1, Vec2 size1, Vec2 pos2, Vec2 size2, Vec2 pos_offset = {});

Entity_Identity collide_enemy(Game_Data_Pointers game_data, Vec2 pos, Vec2 size, Vec2 pos_offset = {});

bool32 collide_wall(Game_Data_Pointers game_data, Vec2 pos, Vec2 size, Vec2 pos_offset = {});
Rectangle collide_wall_rect(Game_Data_Pointers game_data, Vec2 pos, Vec2 size, Vec2 pos_offset = {});
Collide_Data move_collide_wall(Game_Data_Pointers game_data, Vec2* pos, Vec2* spd, Vec2 size);

bool32 on_screen(Vec2 pos, Vec2 padding = {});
