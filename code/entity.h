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
    void Draw(Game_Data_Pointers game_data);
};

#define ENEMY_MAX 32
struct Enemys
{
    int8 count;
    Vec2     pos[ENEMY_MAX];
    Vec2     spd[ENEMY_MAX];
    Vec2     size[ENEMY_MAX];
    My_Color color[ENEMY_MAX];
    
    Enemys* Create(Vec2 _pos, Vec2 _size, My_Color _color);
    void Update(Game_Data_Pointers game_data);
    void Draw(Game_Data_Pointers game_data);
};

#define BULLET_MAX 4
typedef int8 Entity_ID;
struct Bullets
{
    Entity_ID count_total;
    Entity_ID count_alive;
    Entity_ID count_clean;

    Entity_ID id[BULLET_MAX];
    Entity_ID index[BULLET_MAX];
    
    bool32   marked_to_clean[BULLET_MAX];
    bool32   is_initialized[BULLET_MAX]; //TODO: Superceded by alive array?
    Vec2     pos[BULLET_MAX];
    Vec2     spd[BULLET_MAX];
    Vec2     size[BULLET_MAX];
    My_Color color[BULLET_MAX];

    
    Bullets* Create(Vec2 _pos, Vec2 _spd);
    void Update(Game_Data_Pointers game_data);
    void Draw(Game_Data_Pointers game_data);
//     void Clean(Entity_ID index);
//     void Destroy(Entity_ID alive_index);
    void Cleanup_End_Frame();
};
