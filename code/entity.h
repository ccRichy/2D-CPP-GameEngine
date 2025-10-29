/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */
//#pragma once
// #include "game.h"



enum class Ent_Type
{
    Null = -1,
    
    Player,
    Wall,
    Enemy,
    
    Num,
};

struct Entity
{
    //base
    Ent_Type type;
    bool32   is_alive;

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



struct Ent_Info {
    const char* name;
    int max_count;
};

constexpr Ent_Info ENT_INFO[] = {
    { "player", 0 },
    { "wall",   8 },
    { "enemy",  8 },
};

constexpr int ENT_MAX_ALL(){
    i32 result = 0;
    for (int i = 0; i < (i32)Ent_Type::Num; ++i){
        result += ENT_INFO[i].max_count;
    }
    return result;
}
constexpr int ENT_MAX(Ent_Type type){
    i32 result = ENT_INFO[(i32)type].max_count;
    return result;
}


// globalvar const char* global_ent_names[Ent_Type::Num] =
// {
//     "player",
//     "wall",
//     "enemy"
// };
// constexpr int ENT_MAX_COUNTS[] = {
//     0,   // Player
//     8,   // Wall
//     8,   // Enemy
// };
