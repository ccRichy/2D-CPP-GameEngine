/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */




inline int32
entity_get_num(Ent_Type type)
{
    int32 result = pointers->entity->nums[(i32)type];
    return result;
}

Sprite* entity_sprite_default(Ent_Type type)
{
    Sprite* result = nullptr;
    Game_Sprites* spr = GSPRITE;
    switch (type)
    {
        case Ent_Type::Player: result = &spr->sPlayer_idle; break;
        case Ent_Type::Wall: result = &spr->sWall_anim; break;
        case Ent_Type::Enemy: result = &spr->sBlob_small; break;
    }
    return result;
}

void
entity_clear_all()
{
    Entity* array = pointers->entity->array;
    
    for (int ent_index = 0; ent_index < ENT_MAX_ALL(); ++ent_index){
        //DEBUG: HACK:
        Ent_Type type_saved = array[ent_index].type;
        array[ent_index] = {};
        array[ent_index].type = type_saved;
    }
    
    for (i32 ent_num_index = 0; ent_num_index < (i32)Ent_Type::Num; ++ent_num_index)
        pointers->entity->nums[ent_num_index] = 0;
}

inline void
entity_draw_default(Entity* entity)
{
    draw_sprite_frame(
        entity->sprite,
        entity->pos,
        entity->anim_index,
        entity->scale
    );
}


void
entity_destroy(Entity* entity)
{
    if (entity)
    {
        entity->is_alive = false;
        pointers->entity->nums[(int32)entity->type] -= 1;
    }
}


Entity*
entity_init(Ent_Type type, Vec2f pos)
{
    Entity* array = ENT_POINT(type);
    i32 num_of_alive = entity_get_num(type);//pointers->entity->nums[(int32)type];
    i32 array_max = ENT_MAX(type); 
    if (num_of_alive >= array_max) return nullptr;
    
    Entity* result = nullptr;
    for (int i = 0; i < array_max; ++i)
    {
        int32 looped_index = (num_of_alive+i) % array_max;
        if (!array[looped_index].is_alive)
        {
            result = &array[looped_index]; 
            result->sprite = entity_sprite_default(type); //set default sprite
            result->type = type;
            result->pos = pos;
            result->is_alive = true;
            pointers->entity->nums[(i32)type]++;
            break;
        }
    }    
    return result;
}

//
Entity*
wall_create(Vec2f pos, Vec2f size)
{
    Entity* ent = entity_init(Ent_Type::Wall, pos);
    if (ent){
        ent->size = size;
        ent->color = RAYWHITE;
    }
    return ent;
};
void
wall_draw()
{
    for (int i = 0; i < ENT_MAX(Ent_Type::Wall); ++i)
    {
        Entity* ent = &ENT_POINT(Ent_Type::Wall)[i];
        if (!ent->is_alive) continue;
        draw_rect(ent->pos, ent->size, ent->color);
    }
}


//
Entity*
enemy_create(Vec2f pos)
{
    Entity* ent = entity_init(Ent_Type::Enemy, pos);
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
    for (int i = 0; i < ENT_MAX(Ent_Type::Enemy); ++i){
        Entity* ent = &ENT_POINT(Ent_Type::Enemy)[i];
        if (!ent->is_alive) continue;
        
        float32 coll_xoffset = (ent->size.x * sign(ent->spd.x) + ent->spd.x);
        ent->pos += ent->spd;
        ent->pos.x++;

        // if (!collide_wall(ent->pos, ent->size, {coll_xoffset, 1}))
        //     spd.x *= -1;

        if (ent->hp <= 0){
            // enemy_die(ent);
        }
    }    
}
void
enemy_draw()
{
    for (int i = 0; i < ENT_MAX(Ent_Type::Enemy); ++i){
        Entity* ent = &ENT_POINT(Ent_Type::Enemy)[i];
        if (!ent->is_alive) continue;
        
        draw_rect(ent->pos, ent->size, ent->color);
    }
}


Entity*
spike_create(Vec2f pos)
{
    Entity* ent = entity_init(Ent_Type::Spike, pos);
    if (ent){
        sprite_set(ent, sSpike);
        ent->size = {8, 8};
    }
    return ent;
}
void spike_draw()
{
    for (int i = 0; i < ENT_MAX(Ent_Type::Spike); ++i){
        Entity* ent = &ENT_POINT(Ent_Type::Spike)[i];
        if (!ent->is_alive) continue;
        
        entity_draw_default(ent);

        IF_DEBUG {
            draw_rect(ent->pos, ent->size, RED);
        }
    }
}
