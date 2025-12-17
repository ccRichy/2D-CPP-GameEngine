/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */

Ent_Type
entity_get_type_with_string(char* ent_name)
{
    Ent_Type result = Ent_Type::Null;
    for (i32 it = 0; it < (i32)Ent_Type::Num; ++it)
    {
        result = (Ent_Type)it;
        if (string_equals(ent_name, ENT_NAME(result)))
            break;
    }
    
    return result;
}

Sprite*
entity_sprite_default(Ent_Type type)
{
    Sprite* result = &GSPRITE->sNull;
    
    #define XMAC(__name, __max_count, __sprite) case Ent_Type::__name: result = &GSPRITE->__sprite; break;
    switch (type){
        ENT_LIST
    }
    #undef XMAC
    
    return result;
}

Rectangle
entity_collision_mask_default(Ent_Type type)
{
    using enum Ent_Type;
    Rect result = {};
    switch (type)
    {
      case Spike:  result = { .pos = {2, 2}, .size = {3, 6} }; break;
      case Enemy:  result = { .pos = {}, .size = {Tile(1), Tile(2)} }; break;
      case Turtle: result = { .pos = {0, 3}, .size = {8, 5} }; break;
        
      default:{
          Sprite* spr = entity_sprite_default(type);
          result = { .size = {.x = (f32)(spr->width / spr->frame_num), .y = (f32)spr->height} };
      }break;
    }
    return result;
}

void entity_draw_type(Ent_Type type);
void entity_draw_default(){
    using enum Ent_Type;

    for (int it = 0; it < (i32)Ent_Type::Num; ++it){
        Ent_Type type = (Ent_Type)it;
        
        switch (type)
        {
          case Player: break;
        
          default: {
              entity_draw_type(type);
          } break;
        }
    }    
}


Entity*
entity_init(Ent_Type type, Vec2f pos)
{
    Entity* array = ENT_POINT(type);
    i32 num_of_alive = ENT_NUM(type);
    i32 ent_max = ENT_MAX(type); 
    if (num_of_alive >= ent_max) return nullptr;

    //find an open index space
    Entity* result = nullptr;
    for (int i = 0; i < ent_max; ++i){
        //start from the 1st hypothetical free index.
        //if an index is found, break;
        //if not, linearly traverse all possible indexes (mod(%). this gives us much greater odds of finding the most likely spot(s) in the fewest loops in a simple way.
        int32 looped_index = (num_of_alive+i) % ent_max;
        if (!array[looped_index].is_alive){
            result = &array[looped_index];
            
            result->sprite = entity_sprite_default(type);
            
            result->bbox = entity_collision_mask_default(type);
            result->type = type;
            result->pos = pos;
            result->is_alive = true;
            pointers->entity->nums[(i32)type]++;
            break;
        }
    }    
    return result;
}
void
entity_destroy(Entity* entity)
{
    if (entity){
        entity->is_alive = false;
        pointers->entity->nums[(int32)entity->type] -= 1;
    }
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

inline bool32
entity_anim_update(Entity* ent)
{
    b32 result = false;
    f32 index_add = (f32)ent->sprite->fps/FPS_TARGET;
    ent->anim_index += index_add * ent->anim_speed;

//method1
    ent->anim_ended_this_frame = false;
    if (ent->anim_index + index_add >= ent->sprite->frame_num) ent->anim_ended_this_frame = true;
    ent->anim_index = mod_f32(ent->anim_index, (f32)ent->sprite->frame_num);
    
//method2
    // if (anim_index >= sprite->frame_num) {
    //     anim_ended_this_frame = true;
    //     result = true;
    //     anim_index = 0;
    // }
    
    return result;
}

inline void
entity_draw_sprite(Entity* ent)
{
    entity_anim_update(ent);
    draw_sprite_frame(
        ent->sprite,
        ent->pos,
        ent->anim_index,
        ent->scale
    );
}

void entity_draw_type(Ent_Type type)
{
    Entity* ent_start = ENT_POINT(type);
    i32 max = ENT_MAX(type);
    for (int it = 0; it < max; ++it)
    {
        Entity* ent = &ent_start[it];
        if (ent->is_alive){
            entity_draw_sprite(ent);
            IF_DEBUG {
                draw_rect(ent->pos + ent->bbox.pos, ent->bbox.size, BBOXRED);
            }
        }
    }
}    



void
entity_state_enter_default(Entity* ent, Sprite* spr, f32 anim_index = 0, f32 anim_speed = 1)
{
    ent->sprite = spr;
    ent->anim_index = anim_index;
    ent->anim_speed = anim_speed;
}




//ENTITY SPECIFIC
Entity*
wall_create(Vec2f pos, Vec2f size)
{
    Entity* ent = entity_init(Ent_Type::Wall, pos);
    if (ent){
        ent->bbox = Rect{ .pos ={}, .size = size };
        ent->color = RAYWHITE;
    }
    return ent;
};
// void
// wall_draw()
// {
//     for (int i = 0; i < ENT_MAX(Ent_Type::Wall); ++i)
//     {
//         Entity* ent = &ENT_POINT(Ent_Type::Wall)[i];
//         if (!ent->is_alive) continue;
//         draw_rect(ent->pos + ent->bbox.pos, ent->bbox.size, ent->color);
//     }
// }


//
Entity*
enemy_create(Vec2f pos)
{
    Entity* ent = entity_init(Ent_Type::Enemy, pos);
    if (ent){
        ent->bbox = { .pos = {}, .size = {Tile(1), Tile(2)} };
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
        
        // float32 coll_xoffset = (ent->size.x * sign(ent->spd.x) + ent->spd.x);
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
bouncy_turtle_update()
{
    for (i32 i = 0; i < ENT_MAX(Ent_Type::Turtle); ++i){
        Entity* ent = &ENT_POINT(Ent_Type::Turtle)[i];
        if (!ent->is_alive) continue;

        //TODO: not working, fix after player is an entity
        Entity* plr = PLAYER;
        f32 ent_top_pos = bbox_top(ent)+1;
        draw_pixel({ent->pos.x - 8, ent_top_pos}, WHITE);
        b32 player_collision = collide_entitys(plr, ent, {.y = plr->spd.y});
        b32 player_is_above = bbox_bottom(plr) <= ent_top_pos;
        if (player_collision && player_is_above){
            plr->y = ent_top_pos-1;
            plr->player_state_switch(Player_State::Jump);
            plr->spd.y = -(plr->jump_spd * 1.28f);
            if (plr->move_input.x == 0)
                plr->spd.x *= 0.4;
        }
    }
}
