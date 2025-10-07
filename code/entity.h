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
globalvar const char* Ent_Names[] =
{
    "player",
    "wall",
    "enemy"
};

#define ENEMY_MAX 8
#define WALL_MAX 8

#define ENT_MAX ENEMY_MAX + WALL_MAX

struct Entity
{
    //base
    Ent_Type type;
    Vec2 pos;
    Vec2 size;
    Vec2 spd;
    Color color;

    //combat
    float32 hp;
};
