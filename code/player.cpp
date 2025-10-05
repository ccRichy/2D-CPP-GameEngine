/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */

#include "player.h"


void
player_spd_hori(Player* plr, bool32 is_airborne)
{
    float32 spd_target = plr->move_input.x * plr->ground_speed_max;
    if (!is_airborne)
    {
        float32 spd = plr->move_input.x != 0 ? plr->physics.accel : plr->physics.decel;
        plr->spd.x = approach(plr->spd.x, spd_target, plr->physics.accel);
    }
    else
    {
        float32 spd = plr->move_input.x != 0 ? plr->physics.accel : plr->physics.decel;
        plr->spd.x = lerp(plr->spd.x, spd_target, plr->physics.accel);
    }
}
void
player_gravity(Player* plr)
{
    plr->spd.y += plr->physics.grav;
}



#if 0
/////STATES
void
state_change(Game_Pointers* game_pointers, State** state_variable, State* target)
{
    *state_variable = target;
    target->Enter(game_pointers);
}
void state_enter_default(Player* plr, Sprite* sprite, float32 anim_index = 0, float32 anim_speed_mult = 1)
{
    plr->sprite = sprite;
    plr->anim_index = anim_index;
    plr->anim_speed_mult = anim_speed_mult;
}


//IDLE
void player_idle_enter(Game_Pointers* game_pointers)
{
    Player* plr = game_pointers->player;
    state_enter_default(plr, &game_pointers->data->sPlayer_idle, 0, 0);
    plr->physics = plr->ground_physics;
}
void
player_idle_step(Game_Pointers* game_pointers, Game_Input_Map input)
{
    Player* plr = game_pointers->player;
    //anim
    plr->anim_index = (float32)input.up.hold;
    //speed
    player_gravity(plr);
    player_spd_hori(plr, false);
    move_collide_wall(game_pointers, &plr->pos, &plr->spd, plr->size);
        
    if (input.jump)
        state_change(game_pointers, &plr->state, &plr->st_jump);
    else if (plr->spd.y != 0)
        state_change(game_pointers, &plr->state, &plr->st_fall);
    else if (plr->move_input.x != 0 && plr->spd.x != 0)
        state_change(game_pointers, &plr->state, &plr->st_walk);
}
    
//WALK
void
player_walk_enter(Game_Pointers* game_pointers)
{
    Player* plr = game_pointers->player;
    state_enter_default(plr, &game_pointers->data->sPlayer_walk);
    plr->physics = plr->ground_physics;
}
void
player_walk_step(Game_Pointers* game_pointers, Game_Input_Map input)
{
    Player* plr = game_pointers->player;

    plr->spd.y += plr->physics.grav;
    player_spd_hori(plr, false);
    move_collide_wall(game_pointers, &plr->pos, &plr->spd, plr->size);

    //anim
    if (sign(plr->move_input.x) != sign(plr->spd.x) && plr->move_input.x != 0)
        plr->sprite = &game_pointers->data->sPlayer_turn;
    else if (input.up.hold)
        plr->sprite = plr->sprite = &game_pointers->data->sPlayer_walk_reach;
    else
        plr->sprite = plr->sprite = &game_pointers->data->sPlayer_walk;

    if (plr->move_input.x != 0)
        plr->scale.x = (float32)sign(plr->move_input.x);
    plr->anim_speed_mult = abs_f32(plr->spd.x);

    //state
    if (input.jump)
        state_change(game_pointers, &plr->state, &plr->st_jump);
    else if (plr->spd.y != 0)
        state_change(game_pointers, &plr->state, &plr->st_fall);
    else if (plr->spd.x == 0)
        state_change(game_pointers, &plr->state, &plr->st_idle);
}

//JUMP
void
player_jump_enter(Game_Pointers* game_pointers)
{
    Player* plr = game_pointers->player;
    state_enter_default(plr, &game_pointers->data->sPlayer_air, 0, 0);

    plr->physics = plr->jump_physics;
    plr->spd.y = -plr->jump_spd;
    // plr->grav = plr->grav_low;
}
void
player_jump_step(Game_Pointers* game_pointers, Game_Input_Map input)
{
    Player* plr = game_pointers->player;

    //anim
    float32 yspd_threshold = 0.4f;
    if (abs_f32(plr->spd.y) > yspd_threshold){
        if (input.up.hold){
            plr->sprite = &game_pointers->data->sPlayer_air_reach;
            plr->anim_index = (plr->move_input.x != 0 && plr->move_input.x != plr->scale.x);
        }else{
            plr->sprite = &game_pointers->data->sPlayer_air;
        }
    }else{
        plr->anim_speed_mult = abs_f32(plr->spd.y) * 1.6f;
    }
        
    //speed
    player_gravity(plr);
    player_spd_hori(plr, true);
    if (!input.jump.hold) plr->spd.y *= 0.9f;
    Collide_Data coll = move_collide_wall(game_pointers, &plr->pos, &plr->spd, plr->size);

    //state
    if (plr->spd.y > 0)
        state_change(game_pointers, &plr->state, &plr->st_fall);
}
    
//FALL
void
player_fall_enter(Game_Pointers* game_pointers)
{
    Player* plr = game_pointers->player;
    Sprite* spr = game_pointers->input->up.hold ? &game_pointers->data->sPlayer_air_reach : &game_pointers->data->sPlayer_air;
    float32 index = game_pointers->input->up.hold ? plr->anim_index : 0;
    state_enter_default(plr, spr, index, 0);
        
    plr->physics = plr->fall_physics;
}
void
player_fall_step(Game_Pointers* game_pointers, Game_Input_Map input)
{
    Player* plr = game_pointers->player;

    //anim
    if (input.up.hold){
        plr->sprite = &game_pointers->data->sPlayer_air_reach;
        plr->anim_index = (plr->move_input.x != 0 && plr->move_input.x != plr->scale.x);
    }else{
        plr->sprite = &game_pointers->data->sPlayer_air;
    }
    plr->anim_speed_mult = abs_f32(plr->spd.y) * 0.8f;

        
    player_spd_hori(plr, true);
    player_gravity(plr);
    Collide_Data coll = move_collide_wall(game_pointers, &plr->pos, &plr->spd, plr->size);
        
    if (coll.ydir == 1)
        state_change(game_pointers, &plr->state,
                     plr->spd.x != 0 ? &plr->st_walk : &plr->st_idle);
};
#endif






void
player_create(Game_Pointers* game_pointers, Vec2 _pos)
{
    Player* plr = &game_pointers->entity->player;
    plr->sprite = &game_pointers->data->sPlayer_idle;

    //
    plr->ground_speed_max = 1.f;
    plr->jump_spd = 1.2f;
    plr->ground_physics = {};
    plr->ground_physics =    {0.07f,  0.045f, 0.1f,  0.1f};
    plr->jump_physics =      {0.05f,  0,      0.1f,  0.06f};
    plr->fall_physics =      {0.01f,  0,      0.1f,  0.08f};
    plr->physics = plr->ground_physics;

    plr->grav_default = 0.12f;
    plr->grav_low = 0.08f;
    plr->grav = plr->grav_default;
    plr->terminal_velocity = 4.f;

    //
    plr->pos   = _pos;
    plr->anim_speed_mult = 1;
    plr->size  = {4, Tile(1)-1};
    plr->scale = {1, 1};
    plr->color = GREEN;

#if 0
    // plr->st_idle.Enter = player_idle_enter;
    // plr->st_idle.Step = player_idle_step;
    // plr->st_walk.Enter = player_walk_enter;
    // plr->st_walk.Step = player_walk_step;
    // plr->st_jump.Enter = player_jump_enter;
    // plr->st_jump.Step = player_jump_step;
    // plr->st_fall.Enter = player_fall_enter;
    // plr->st_fall.Step = player_fall_step;
    
    // plr->state = &plr->st_idle;
    // plr->state->Enter(game_pointers);
#endif
}

void
player_update(Game_Pointers* game_pointers, Game_Input_Map input)
{
    Player* plr = &game_pointers->entity->player;
    
    plr->move_input = {(float32)(input.right.hold - input.left.hold),
                       (float32)(input.down.hold - input.up.hold)};

    plr->pos += plr->move_input;
    
    // plr->state->Step(game_pointers, input);
    if (plr->spd.y > plr->terminal_velocity) plr->spd.y = plr->terminal_velocity;
}

void
player_draw(Player* plr, Game_Pointers* game_pointers)
{
    Sprite* spr = plr->sprite;
    int32 frame_size = (int32)(spr->bmp->width / spr->frame_num);

    plr->anim_index += ((float32)spr->fps/FPS_TARGET) * plr->anim_speed_mult;
    if (plr->anim_index >= spr->frame_num) plr->anim_index = 0;
    int32 frame = floor_i32(plr->anim_index);
    
    Vec2 sprite_offset = {-6.f, -5.f};
    draw_bmp_part(game_pointers, plr->sprite->bmp, plr->pos + sprite_offset, plr->scale,
                  frame * frame_size, 0, //pos
                  16, 16);//size

    //DRAW ORIGIN
    // draw_pixel(game_pointers, plr->pos, WHITE);
}




/* DRAW SMILEY FACE
  
    float32 smile_size = 4;
    draw_rect(game_pointers, {pos.x, pos.y}, {smile_size, smile_size * 5}, blue);
    draw_rect(game_pointers, {pos.x - smile_size, pos.y-smile_size}, {smile_size, smile_size}, red);
    draw_rect(game_pointers, {pos.x + smile_size*5, pos.y-smile_size}, {smile_size, smile_size}, red);
    draw_rect(game_pointers, {pos.x, pos.y - (smile_size*3)}, {smile_size, smile_size}, red);
    draw_rect(game_pointers, {pos.x + (smile_size*4), (pos.y-smile_size*3)}, {smile_size, smile_size}, red);

 */
