/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */

#include "entity.h"
#include "render.h"



int32 entity_get_num(Ent_Type type)
{
    return pointers->entity->nums[(int32)type];
}


void entity_destroy(Entity* entity)
{
    entity->is_alive = false;
    pointers->entity->nums[(int32)entity->type] -= 1;
}

Entity* entity_init(Entity* array, int32 array_max, Ent_Type type, Vec2 pos)
{
    auto num_of_alive = pointers->entity->nums[(int32)type];
    if (num_of_alive >= array_max) return nullptr;
    
    Entity* result = nullptr;
    for (int i = 0; i < array_max; ++i)
    {
        int32 index_value = num_of_alive+i;
        int32 looped_index = index_value % array_max;
        if (!array[looped_index].is_alive)
        {
            result = &array[looped_index];
            result->type = type;
            result->pos = pos;
            result->is_alive = true;
            pointers->entity->nums[(int32)type]++;
            break;
        }
    }    
    return result;
}

//
Entity*
wall_create(Vec2 pos, Vec2 size)
{
    Entity* ent = entity_init(
        pointers->entity->walls, WALL_MAX,
        Ent_Type::wall, pos
    );
    if (ent){
        ent->size = size;
        ent->color = RAYWHITE;
    }
    return ent;
};
void
wall_draw()
{
    for (int i = 0; i < WALL_MAX; ++i)
    {
        Entity* ent = &pointers->entity->walls[i];
        if (!ent->is_alive) continue;
        draw_rect(ent->pos, ent->size, ent->color);
    }
}


//
Entity*
enemy_create(Vec2 pos)
{
    Entity* ent = entity_init(
        pointers->entity->enemys, ENEMY_MAX,
        Ent_Type::enemy, pos
    );    
    if (ent){
        ent->size = {Tile(1), Tile(2)};
        ent->color = RED;
    }
    return ent;
}
void
enemy_update()
{
    float32 maxspd = 0.4f;
    for (int i = 0; i < ENEMY_MAX; ++i)
    {
        Entity* ent = &pointers->entity->enemys[i];
        if (!ent->is_alive) continue;
        
        float32 coll_xoffset = (ent->size.x * sign(ent->spd.x) + ent->spd.x);
        ent->pos += ent->spd;

        // if (!collide_wall(ent->pos, ent->size, {coll_xoffset, 1}))
        //     spd.x *= -1;

        if (ent->hp <= 0){
        //TODO: clean
        }
    }    
}
void
enemy_draw()
{
    for (int i = 0; i < ENEMY_MAX; ++i)
    {
        Entity* ent = &pointers->entity->enemys[i];
        if (!ent->is_alive) continue;
        
        draw_rect(ent->pos, ent->size, ent->color);
    }
}


//
// void
// bullet_update(Entity* ent)
// {    
//     ent->pos += ent->spd;

// //        Entity_Identity enemy_id = collide_enemy(ent->pos, ent->size);
//     // Entity_Identity enemy_id = {};
//     // collide_entity(enemy_id, ent->pos, ent->size, game_pointers->entity->enemys);
//     // if (enemy_id)
//     // {
//     //     ent->pos.y -= 16;
//     //     spd.y -= 5;
//     //     game_pointers->entity->enemys.hp[enemy_id.index] -= 1;
//     //     game_pointers->entity->enemys.color[enemy_id.index] = color_mult_value_rgb(game_pointers->entity->enemys.color[enemy_id.index], 0.9f);
//     // }
        
//     if (!on_screen(ent->pos, ent->size))
//     {
//         //TODO: clean
//     }
// }
// void
// bullet_draw(Entity* ent)
// {
//     draw_rect(ent->pos, ent->size, ent->color);
// }
