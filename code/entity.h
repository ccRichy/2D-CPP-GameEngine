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


typedef int32 Entity_ID; //REQUIRED: 0 = NULL
struct Entity_Identity
{
};

#define ENEMY_MAX 8
#define WALL_MAX 8
#define BULLET_MAX 4

struct Entity
{
    //ENTITIY
    //base
    Ent_Type type;
    
    Vec2 pos;
    Vec2 size;
    Vec2 spd;
    Color color;

    //combat
    float32 hp;
};
