/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */
#include "entity.h"
#include "render.h"


Walls* Walls::Create(Vec2 _pos, Vec2 _size, My_Color _color)
{
    pos[count] = _pos;
    size[count] = _size;
    color[count] = _color;

    count++;
    return this;
}
void Walls::Draw(Game_Data_Pointers game_data)
{
    for (int i = 0; i < count; ++i)
    {
        draw_rect(game_data, pos[i], size[i], color[i]);
    }
}


Enemys* Enemys::Create(Vec2 _pos, Vec2 _size, My_Color _color)
{
    pos[count] = _pos;
    size[count] = _size;
    color[count] = _color;

    spd[count].x = 0.4f;
    
    count++;
    return this;
}
void Enemys::Update(Game_Data_Pointers game_data)
{
    float32 maxspd = 0.4f;
    for (int i = 0; i < count; ++i)
    {
        float32 coll_xoffset = (size[i].x * sign(spd[i].x) + spd[i].x);
        if (!collide_wall(game_data, pos[i], size[i], {coll_xoffset, 1}))
            spd[i].x *= -1;
        
        pos[i] += spd[i];
    }
}
void Enemys::Draw(Game_Data_Pointers game_data)
{
    for (int i = 0; i < count; ++i)
    {
        draw_rect(game_data, pos[i], size[i], color[i]);
    }
}


Bullets* Bullets::Create(Vec2 _pos, Vec2 _spd)
{
    Entity_ID wrapped_i = count_total % (BULLET_MAX); //actual array index position

    Entity_ID target_i = wrapped_i;
    if (is_initialized[target_i])
    {
        for (Entity_ID i = 0; i < BULLET_MAX; ++i) 
        {
            Entity_ID next_i = (i + target_i + 1) % (BULLET_MAX); //check if slots are open
            if (!is_initialized[next_i])
                target_i = next_i;
        }
        if (target_i == wrapped_i){ //couldnt find a slot
            //TODO: Log? Crash? Depends on the desired behavior, perhaps its a need-to-care basis
            return this;
        }
    }
    
    pos[target_i] = _pos;
    spd[target_i] = _spd;

    size[target_i] = {4, 4};
    color[target_i] = blue;

    id[target_i] = count_total;
    index[target_i] = target_i;
//    alive[count_alive] = index[target_i];
    is_initialized[target_i] = true;
    count_alive++;
    count_total++;
    return this;
}
void Bullets::Update(Game_Data_Pointers game_data)
{
    // float32 maxspd = 3.0f;
    for (int8 i = 0; i < BULLET_MAX; ++i)
    {
        Entity_ID ai = i;
        if (!is_initialized[i]) continue;
        
        pos[ai] += spd[ai];
        
        if (pos[ai].x > 200)
            marked_to_clean[ai] = true;
    }
}
void Bullets::Draw(Game_Data_Pointers game_data)
{
    for (Entity_ID i = 0; i < BULLET_MAX; ++i)
    {
        Entity_ID ai = i;
        if (!is_initialized[i]) continue;
        
        draw_rect(game_data, pos[ai], size[ai], color[ai]);
    }
}


// void Bullets::Clean(Entity_ID _index)
// {
//     marked_to_clean[_index] = true;    
//     count_clean++;
// }

// void Bullets::Destroy(Entity_ID alive_index)
// {
//     count_alive--;
//     Entity_ID real_index = alive[alive_index];
//     is_initialized[real_index] = false;

//     alive[alive_index] = 0;
//     // if (_index == count_alive)
//     //     return;
//     for (Entity_ID i = alive_index; i+1 < BULLET_MAX; ++i)
//     {
//         Entity_ID id_to_move_back = alive[i+1];
//         alive[i] = id_to_move_back;
//     }
//     // while (i+1 < BULLET_MAX) //NOTE: this currently automatically zeros all further indexes
//     // {                        //we could make BULLET_MAX count_alive to save loops,
//     //     Entity_ID id_to_move_back = alive[i+1]; //but leaving it as-is, is visually useful
//     //     alive[i] = id_to_move_back;
//     //     i++;
//     // }
//     // if (index+1 < BULLET_MAX)
//     //     index+1
// }

void Bullets::Cleanup_End_Frame()
{
    for (int i = 0; i < BULLET_MAX; ++i)
    {
        if (marked_to_clean[i])
        {
            is_initialized[i] = false;
            count_alive--;
        }
    }
}
