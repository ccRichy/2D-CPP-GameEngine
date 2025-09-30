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
    int32 xdir;
    int32 ydir;
    Vec2 spd;
};


//REQUIRED: __return_value must be initialized first
#define collide_entity(__return_value, __pos, __size, __entity_struct)\
for (Entity_ID __it = 0; __it < __entity_struct.count_alive; ++__it)  \
{                                                                     \
    Entity_ID __ai = __entity_struct.alive[__it];                     \
    Vec2 __pos_target = __entity_struct.pos[__ai];                    \
    Vec2 __size_target = __entity_struct.size[__ai];                  \
    if (collide(__pos, __size, __pos_target, __size_target))          \
    {                                                                 \
        __return_value = __entity_struct.identity[__ai];              \
        break;                                                        \
    }                                                                 \
}                                                                     

#define collide_entity_rect(__return_value, __pos, __size, __entity_struct)\
for (Entity_ID __it = 0; __it < __entity_struct.count_alive; ++__it)       \
{                                                                          \
    Entity_ID __ai = __entity_struct.alive[__it];                          \
    Vec2 __pos_target = __entity_struct.pos[__ai];                         \
    Vec2 __size_target = __entity_struct.size[__ai];                       \
    if (collide(__pos, __size, __pos_target, __size_target))               \
    {                                                                      \
        __return_value = {__pos_target[ai], __size_target[ai]};            \
        break;                                                             \
    }                                                                      \
}                                                                     




bool32 collide(Vec2 pos1, Vec2 size1, Vec2 pos2, Vec2 size2, Vec2 pos_offset = {});
    
Entity_Identity collide_enemy(Game_Pointers game_pointers, Vec2 pos, Vec2 size, Vec2 pos_offset = {});

bool32 collide_wall(Game_Pointers game_pointers, Vec2 pos, Vec2 size, Vec2 pos_offset = {});
Rectangle collide_wall_rect(Game_Pointers game_pointers, Vec2 pos, Vec2 size, Vec2 pos_offset = {});
Collide_Data move_collide_wall(Game_Pointers game_pointers, Vec2* pos, Vec2* spd, Vec2 size);

bool32 on_screen(Vec2 pos, Vec2 padding = {});
