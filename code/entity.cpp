/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */
#include "entity.h"
#include "render.h"



//
Walls* Walls::Create(Vec2 _pos, Vec2 _size, My_Color _color)
{
    pos[count_alive] = _pos;
    size[count_alive] = _size;
    color[count_alive] = _color;

    count_alive++;
    return this;
}
void Walls::Draw(Game_Data_Pointers game_data)
{
    for (int i = 0; i < count_alive; ++i)
    {
        draw_rect(game_data, pos[i], size[i], color[i]);
    }
}



//
Enemys* Enemys::Create(Vec2 _pos, Vec2 _size, My_Color _color)
{
    pos[count_alive] = _pos;
    size[count_alive] = _size;
    color[count_alive] = _color;

    hp[count_alive] = 2;
    spd[count_alive].x = 0.4f;

    identity[count_alive] = {count_alive, count_alive};
    
    count_alive++;
    return this;
}
void Enemys::Update(Game_Data_Pointers game_data)
{
    float32 maxspd = 0.4f;
    for (int i = 0; i < count_alive; ++i)
    {
        float32 coll_xoffset = (size[i].x * sign(spd[i].x) + spd[i].x);
        if (!collide_wall(game_data, pos[i], size[i], {coll_xoffset, 1}))
            spd[i].x *= -1;
        
        pos[i] += spd[i];

        if (hp <= 0)
            Clean(identity[i]);
    }
}
void Enemys::Draw(Game_Data_Pointers game_data)
{
    for (int i = 0; i < count_alive; ++i)
    {
        draw_rect(game_data, pos[i], size[i], color[i]);
    }
}



// void
// entity_is_valid(Entity_Identity identity)
// {
//     if ()
// }

//
Entity_Identity Bullets::Create(Vec2 _pos, Vec2 _spd)
{   
    if (count_alive >= BULLET_MAX) return {-1, -1};

    Entity_ID wrapped_i = count_total % (BULLET_MAX); //actual array index position
    Entity_ID target_i = wrapped_i;
    if (is_initialized[target_i]) //slot already in use
    {
        for (int i = 0; i < BULLET_MAX; ++i) //loop through all slots (from position)
        {
            Entity_ID next_index = (i + target_i + 1) % (BULLET_MAX); //wrap
            if (!is_initialized[next_index]) //if free, use that
                target_i = next_index;
        }
        if (target_i == wrapped_i) //NOTE: this should never happen because of the early out
            return {-1, -1};
    }
     
    is_initialized[target_i] = true;
    index[target_i] = target_i;
    alive[count_alive] = target_i;
    
    pos[target_i] = _pos;
    spd[target_i] = _spd;
    
    size[target_i] = {4, 4};
    color[target_i] = blue;

    count_alive++;
    count_total++;
    return {count_alive-1, target_i};
}
void Bullets::Update(Game_Data_Pointers game_data)
{
    // float32 maxspd = 3.0f;
    for (int8 i = 0; i < count_alive; ++i)
    {
        Entity_ID ai = alive[i];
        
        pos[ai] += spd[ai];

//        collide_entity(enemy_coll, game_data.state->enemys, pos[ai], size[ai], {});
        Entity_Identity enemy_id = collide_enemy(game_data, pos[ai], size[ai]);
        if (enemy_id.id != -1) //TODO: validity check
        {
            spd[ai].y -= 5;
            game_data.state->enemys.hp[enemy_id.index] -= 1;
        }
        
        if (on_screen(pos[ai], size[ai]))
            Clean(index[ai]);
    }
}
void Bullets::Draw(Game_Data_Pointers game_data)
{
    for (Entity_ID i = 0; i < count_alive; ++i)
    {
        Entity_ID ai = alive[i];
        
        draw_rect(game_data, pos[ai], size[ai], color[ai]);
    }
}

void Bullets::Clean(Entity_ID _index) //real-index
{
    marked_for_clean[_index] = true;
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
            is_initialized[real_index] = false;
            marked_for_clean[real_index] = false;
            count_alive--;
        }
    }
    count_clean = 0;
}
