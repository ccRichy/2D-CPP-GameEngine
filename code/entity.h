/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */
#pragma once




#include "game.h"




enum class Ent_Type
{
    Null = -1,
    
    Player,
    Wall,
    Enemy,
    
    Num,
};
globalvar const char* global_ent_names[Ent_Type::Num] =
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

    Sprite* sprite;
    float32 anim_index;
    Vec2f   scale;
    
    Vec2f pos;
    Vec2f size;
    Vec2f spd;
    Color color;

    //combat
    float32 hp;
};
