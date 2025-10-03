/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */
#include "entity.h"
#include "render.h"


//
Entity*
wall_create(Game_Pointers* game_pointers, Vec2 pos, Vec2 size)
{
    Entity* wall = &game_pointers->entity->walls[game_pointers->entity->wall_num];
    wall->pos = pos;
    wall->size = size;
    wall->color = RAYWHITE;
    game_pointers->entity->wall_num++;
    return wall;
};
void
wall_draw(Game_Pointers* game_pointers)
{
    for (int i = 0; i < game_pointers->entity->wall_num; ++i)
    {
        Entity* ent = &game_pointers->entity->walls[i];
        draw_rect(game_pointers, ent->pos, ent->size, ent->color);
    }
}


//
Entity*
enemy_create(Game_Pointers* game_pointers, Vec2 pos)
{
    Entity* enemy = &game_pointers->entity->enemys[game_pointers->entity->enemy_num];
    enemy->pos = pos;
    enemy->size = {Tile(1), Tile(2)};
    enemy->color = RED;

    game_pointers->entity->enemy_num++;
    return enemy;
}
void
enemy_update(Game_Pointers* game_pointers)
{
    float32 maxspd = 0.4f;
    for (int i = 0; i < game_pointers->entity->wall_num; ++i)
    {
        Entity* ent = &game_pointers->entity->walls[i];
        float32 coll_xoffset = (ent->size.x * sign(ent->spd.x) + ent->spd.x);

        ent->pos += ent->spd;

        // if (!collide_wall(game_pointers, ent->pos, ent->size, {coll_xoffset, 1}))
        //     spd.x *= -1;

        if (ent->hp <= 0){
        //TODO: clean
        }
    }    
}
void
enemy_draw(Game_Pointers* game_pointers)
{
    for (int i = 0; i < game_pointers->entity->enemy_num; ++i)
    {
        Entity* ent = &game_pointers->entity->enemys[i];
        draw_rect(game_pointers, ent->pos, ent->size, ent->color);
    }
}


//
void
bullet_update(Entity* ent, Game_Pointers* game_pointers)
{    
    ent->pos += ent->spd;

//        Entity_Identity enemy_id = collide_enemy(game_pointers, ent->pos, ent->size);
    // Entity_Identity enemy_id = {};
    // collide_entity(enemy_id, ent->pos, ent->size, game_pointers->entity->enemys);
    // if (enemy_id)
    // {
    //     ent->pos.y -= 16;
    //     spd.y -= 5;
    //     game_pointers->entity->enemys.hp[enemy_id.index] -= 1;
    //     game_pointers->entity->enemys.color[enemy_id.index] = color_mult_value_rgb(game_pointers->entity->enemys.color[enemy_id.index], 0.9f);
    // }
        
    if (!on_screen(ent->pos, ent->size))
    {
        //TODO: clean
    }
}
void
bullet_draw(Entity* ent, Game_Pointers* game_pointers)
{
    draw_rect(game_pointers, ent->pos, ent->size, ent->color);
}
