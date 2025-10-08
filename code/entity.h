#pragma once

#include "game.h"

enum class Ent_Type
{
    player,
    wall,
    enemy,
    
    num,
    null
};
globalvar const char* global_ent_names[Ent_Type::num] =
{
    "player",
    "wall",
    "enemy"
};

//TODO: consider wether or not to make these MAX values into a parallel array
//      so we can access them dynamically based on Ent_Type
#define ENEMY_MAX 8
#define WALL_MAX 8

#define ENT_MAX ENEMY_MAX + WALL_MAX

struct Entity
{
    //base
    Ent_Type type;
    bool32 is_alive;
    
    Vec2 pos;
    Vec2 size;
    Vec2 spd;
    Color color;

    //combat
    float32 hp;
};

// struct Entity_Data
// {
//     Entity* array_pointers[Ent_Type::num];
//     const char* names[Ent_Type::num];
//     int32 nums[Ent_Type::num];

//     int32 Num(Ent_Type type)
//     {
//         return nums[(int32)type];
//     }
// };
