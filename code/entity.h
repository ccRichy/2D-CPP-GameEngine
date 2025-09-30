#pragma once

//#include "my_vec.h"
#include "game.h"
//#include "my_color.h"


//TODO: Maybe consider an Entity 'component' aka another struct that just includes
//      the common elements so we can work on all of that data together


enum class Ent_Type
{
    player,
    wall,
    enemy,
    null
};


typedef int32 Entity_ID; //REQUIRED: 0 = NULL
struct Entity_Identity
{
    Entity_ID id;
    Entity_ID index;

    explicit operator bool() const noexcept {
        return id != 0;
    }
};


//NOTE: validation is ONLY necessary when referencing a previously STORED identity
//TODO: test this further
#define is_entity_valid(__entity_struct, __entity_identity)\
(__entity_struct.identity[__entity_identity.index].id == __entity_identity.id)

#define entity_spawn(__entity_struct, __pos) game_pointers->entity->__entity_struct.Create(__pos)


#define WALL_MAX 32
struct Walls
{
    int8  count_alive;
    
    Vec2     pos[WALL_MAX];
    Vec2     size[WALL_MAX];
    Color    color[WALL_MAX];
    
    Walls* Create(Vec2 _pos, Vec2 _size);
    void Draw(Game_Pointers game_pointers);
};

#define ENEMY_MAX 32
struct Enemys
{
    Entity_ID count_alive;
    Entity_ID count_total;
    Entity_ID count_clean;
    
    Entity_Identity identity[ENEMY_MAX];
    Entity_ID alive[ENEMY_MAX];
    bool32   marked_for_clean[ENEMY_MAX];
    
    Vec2      pos[ENEMY_MAX];
    Vec2      spd[ENEMY_MAX];
    Vec2      size[ENEMY_MAX];
    Color     color[ENEMY_MAX];

    int8      hp[ENEMY_MAX];
    
    Enemys* Create(Vec2 _pos);
    void Update(Game_Pointers game_pointers);
    void Draw(Game_Pointers game_pointers);
    void Clean(Entity_Identity _identity);
    void Cleanup_End_Frame();
};

#define BULLET_MAX 4
struct Bullets
{
    Entity_ID count_total;
    Entity_ID count_alive;
    Entity_ID count_clean;

    Entity_Identity identity[BULLET_MAX];
    Entity_ID alive[BULLET_MAX];
    bool32 marked_for_clean[BULLET_MAX];
    
    Vec2     pos[BULLET_MAX];
    Vec2     spd[BULLET_MAX];
    Vec2     size[BULLET_MAX];
    Color    color[BULLET_MAX];
    
    Entity_Identity Create(Vec2 _pos, Vec2 _spd);
    void Update(Game_Pointers game_pointers);
    void Draw(Game_Pointers game_pointers);
    void Clean(Entity_Identity _index);
    void Cleanup_End_Frame();
};
