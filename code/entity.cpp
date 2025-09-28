/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */
#include "entity.h"
#include "render.h"



//agnostic
Entity_ID entity_find_open_slot(Entity_ID first_target_index,
                             Entity_Identity* identity_array, Entity_ID array_length)
{
    if (identity_array[first_target_index]) //slot already in use
    {
        for (int i = 0; i < array_length; ++i) //loop through all slots (from position)
        {
            Entity_ID next_index = (i + first_target_index + 1) % (array_length); //wrap
            if (!identity_array[next_index]) //if free, use that
                return next_index;
        }
    }
    return first_target_index;
}




//
Walls* Walls::Create(Vec2 _pos, Vec2 _size)
{
    pos[count_alive] = _pos;
    size[count_alive] = _size;
    color[count_alive] = WHITE;

    count_alive++;
    return this;
}
void Walls::Draw(Game_Pointers game_pointers)
{
    for (int i = 0; i < count_alive; ++i)
    {
        draw_rect(game_pointers, pos[i], size[i], color[i]);
    }
}



//
Enemys* Enemys::Create(Vec2 _pos)
{
    if (count_alive >= ENEMY_MAX) return {};

    Entity_ID wrapped_i = count_total % ENEMY_MAX; //actual array index position
    Entity_ID target_i = entity_find_open_slot(wrapped_i, identity, ENEMY_MAX);
    if (identity[target_i])
        return {};
    
    alive[count_alive] = target_i;
    identity[target_i] = {target_i+1, target_i};

    pos[target_i] = _pos;

    color[target_i] = RED;
    size[target_i] = {Tile(1), Tile(2)};
    hp[target_i] = 3;
    spd[target_i].x = 0.4f;
    
    count_alive++;
    count_total++;
    return this;
}
void Enemys::Update(Game_Pointers game_pointers)
{
    float32 maxspd = 0.4f;
    for (int i = 0; i < count_alive; ++i)
    {
        Entity_ID ai = alive[i];

        float32 coll_xoffset = (size[ai].x * sign(spd[ai].x) + spd[ai].x);
        if (!collide_wall(game_pointers, pos[ai], size[ai], {coll_xoffset, 1}))
            spd[ai].x *= -1;
        
        pos[ai] += spd[ai];

        if (hp[ai] <= 0)
            Clean(identity[ai]);
    }
}
void Enemys::Draw(Game_Pointers game_pointers)
{
    for (int i = 0; i < count_alive; ++i)
    {
        Entity_ID ai = alive[i];
        draw_rect(game_pointers, pos[ai], size[ai], color[ai]);

        Vec2 hp_pos_offset = {0, -8};
        Vec2 hp_length = {4.0f * hp[ai], 2.0f};
        draw_rect(game_pointers, pos[ai] + hp_pos_offset, hp_length, color[ai]);
    }
}
void Enemys::Clean(Entity_Identity _identity)
{
    marked_for_clean[_identity.index] = true;
    count_clean++;
}
void Enemys::Cleanup_End_Frame()
{
    if (count_clean <= 0) return;
    
    for (int i = count_alive-1; i >= 0; --i) //loop backwards to prevent skip
    {
        Entity_ID real_index = alive[i];
        if (marked_for_clean[real_index])
        {
            array_erase_index(alive, ENEMY_MAX, i, 0);
            identity[real_index] = {};
            marked_for_clean[real_index] = false;
            count_alive--;
        }
    }
    count_clean = 0;
}



//
Entity_Identity Bullets::Create(Vec2 _pos, Vec2 _spd)
{   
    if (count_alive >= BULLET_MAX) return {};

    Entity_ID wrapped_i = count_total % BULLET_MAX; //actual array index position
    Entity_ID target_i = entity_find_open_slot(wrapped_i, identity, BULLET_MAX);
    if (identity[target_i])
        return {};

    alive[count_alive] = target_i;
    identity[target_i] = {count_total+1, target_i};

    pos[target_i] = _pos;
    spd[target_i] = _spd;

    size[target_i] = {4, 4};
    color[target_i] = BLUE;

    count_alive++;
    count_total++;

    return identity[target_i];
}
void Bullets::Update(Game_Pointers game_pointers)
{
    // float32 maxspd = 3.0f;
    for (int8 i = 0; i < count_alive; ++i)
    {
        Entity_ID ai = alive[i];
        
        pos[ai] += spd[ai];

//        Entity_Identity enemy_id = collide_enemy(game_pointers, pos[ai], size[ai]);
        Entity_Identity enemy_id = {};
        collide_entity(enemy_id, pos[ai], size[ai], game_pointers.entity->enemys);
        if (enemy_id)
        {
            pos[ai].y -= 16;
            spd[ai].y -= 5;
            game_pointers.entity->enemys.hp[enemy_id.index] -= 1;
            game_pointers.entity->enemys.color[enemy_id.index] = color_mult_value_rgb(game_pointers.entity->enemys.color[enemy_id.index], 0.9f);
        }
        
        if (on_screen(pos[ai], size[ai]))
            Clean(identity[ai]);
    }
}
void Bullets::Draw(Game_Pointers game_pointers)
{
    for (Entity_ID i = 0; i < count_alive; ++i)
    {
        Entity_ID ai = alive[i];
        
        draw_rect(game_pointers, pos[ai], size[ai], color[ai]);
    }
}

void Bullets::Clean(Entity_Identity _identity) //real-index
{
    marked_for_clean[_identity.index] = true;
    count_clean++;
}
void Bullets::Cleanup_End_Frame()
{
    if (count_clean <= 0) return;
    
    for (int i = count_alive-1; i >= 0; --i) //loop backwards to prevent skip
    {
        Entity_ID real_index = alive[i];
        if (marked_for_clean[real_index])
        {
            array_erase_index(alive, BULLET_MAX, i, 0);
            identity[real_index] = {};
            marked_for_clean[real_index] = false;
            count_alive--;
        }
    }
    count_clean = 0;
}
