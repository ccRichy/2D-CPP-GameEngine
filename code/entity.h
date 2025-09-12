#pragma once

#include "my_vec.h"
#include "game.h"
//#include "my_color.h"


//TODO: Maybe consider an Entity 'component' aka another struct that just includes
//      the common elements so we can work on all of that data together


typedef int32 Entity_ID;
struct Entity_Identity //TODO: ELABORATE
{
    Entity_ID id;
    Entity_ID index;

    explicit operator bool() const noexcept {
        return id != -1;
    }
};


//NOTE: the only time validation is necessary is when we are referencing a STORED entity
//TODO: BULLETPROOF THIS!!
#define is_entity_valid(__entity_struct, __entity_identity)\
(__entity_struct.identity[__entity_identity.index].id == __entity_identity.id)



#define WALL_MAX 32
struct Walls
{
    int8 count_alive;
    Vec2     pos[WALL_MAX];
    Vec2     size[WALL_MAX];
    Color color[WALL_MAX];
    
    Walls* Create(Vec2 _pos, Vec2 _size, Color _color);
    void Draw(Game_Data_Pointers game_data);
};

#define ENEMY_MAX 32
struct Enemys
{
    int8     count_alive;
    bool32   marked_for_clean[ENEMY_MAX];
    Entity_Identity identity[ENEMY_MAX];
    
    Vec2     pos[ENEMY_MAX];
    Vec2     spd[ENEMY_MAX];
    Vec2     size[ENEMY_MAX];
    Color    color[ENEMY_MAX];
    int8     hp[ENEMY_MAX];

    
    Enemys* Create(Vec2 _pos, Vec2 _size, Color _color);
    void Update(Game_Data_Pointers game_data);
    void Draw(Game_Data_Pointers game_data);
    void Clean(Entity_Identity _identity);
    void Cleanup_End_Frame();
};

#define BULLET_MAX 4
struct Bullets
{
    bool32 is_initialized[BULLET_MAX];
    bool32 marked_for_clean[BULLET_MAX];
    Entity_ID alive[BULLET_MAX] = {};

    Entity_ID count_total;
    Entity_ID count_alive;
    Entity_ID count_clean;

    Entity_ID index[BULLET_MAX];
    
    Vec2     pos[BULLET_MAX];
    Vec2     spd[BULLET_MAX];
    Vec2     size[BULLET_MAX];
    Color    color[BULLET_MAX];
    
    Entity_Identity Create(Vec2 _pos, Vec2 _spd);
    void Update(Game_Data_Pointers game_data);
    void Draw(Game_Data_Pointers game_data);
    void Clean(Entity_ID _index);
    void Cleanup_End_Frame();
};
