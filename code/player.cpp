/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */

#include "player.h"



// struct State
// {
//     void (*enter)(void);
//     void (*step)(void);
//     void (*leave)(void);
// };

// void player_state_idle_create(Game_Pointers game_pointers)
// {
//     Player* plr = game_pointers.player;
//     plr->sprite = &game_pointers.data->sPlayer_idle;
//     plr->anim_index = 0;
//     plr->anim_speed_mult = 0;
// }
// void player_state_idle_step(Game_Pointers game_pointers)
// {
//    
// }






void
player_create(Game_Pointers* game_pointers, Vec2 _pos)
{
    Player* plr = &game_pointers->entity->player;
    plr->sprite = &game_pointers->data->sPlayer_idle;

    //numbas
    plr->ground_speed_max = 2.f;
    plr->grav_default = 0.11f;
    plr->grav_low = 0.08f;
    plr->grav = plr->grav_default;
    
    plr->state = Player_State::idle;
    plr->pos   = _pos;
    plr->anim_speed_mult = 0;
    plr->size  = {4, Tile(1)-1};
    plr->scale = {1, 1};
    plr->color = GREEN;
}

void
player_move_hori(Player* plr, bool32 is_airborne)
{
    float32 spd_target = plr->move_input.x * plr->ground_speed_max;
    if (!is_airborne)
        plr->spd.x = approach(plr->spd.x, spd_target, 0.04f);
    else
        plr->spd.x = approach(plr->spd.x, spd_target, 0.04f);
}

void
player_update(Game_Pointers* game_pointers, Game_Input_Map input)
{
    Player* plr = &game_pointers->entity->player;
    
    float32 target_spd = 1.f;
    float32 jumpspd = 2.1f;
    float32 max_fall_spd = 4.f;
    
    plr->move_input = {(float32)(input.right.hold - input.left.hold),
                       (float32)(input.down.hold - input.up.hold)};

    if (plr->state == Player_State::idle)
    {
        //sprite
        if (plr->spd.x != 0){
            plr->sprite = &game_pointers->data->sPlayer_walk;
            plr->anim_speed_mult = abs_f32(plr->spd.x);
        }
        else{
            plr->anim_index = (float32)(input.up.hold);
            
            plr->sprite = &game_pointers->data->sPlayer_idle;
            plr->anim_speed_mult = 0;
        }

        //input
        player_move_hori(plr, false);
        if (plr->move_input.x != 0)
            plr->scale.x = (float32)sign(plr->move_input.x);
        
        if (input.jump){
            plr->spd.y = -jumpspd;
            plr->grav = plr->grav_low;
            plr->state = Player_State::jump;
        }

        //
        plr->spd.y += plr->grav;
        Collide_Data coll = move_collide_wall(game_pointers, &plr->pos, &plr->spd, plr->size);
    }
    else if (plr->state == Player_State::jump)
    {
        //sprite
        plr->sprite = &game_pointers->data->sPlayer_walk;
        plr->anim_speed_mult = 0;
        plr->anim_index = 0;

        //spd
        player_move_hori(plr, true);
        if (input.jump){
            plr->spd.y = -jumpspd;
            plr->grav = plr->grav_low;
        }
        else if (input.jump.release){
            if (plr->spd.y < 0)
            {
                plr->spd.y /= 3;
                plr->grav = plr->grav_default;
            }
        }
        plr->spd.y += plr->grav;
        if (plr->spd.y > max_fall_spd) plr->spd.y = max_fall_spd;

        Collide_Data coll = move_collide_wall(game_pointers, &plr->pos, &plr->spd, plr->size);
        if (coll.ydir == 1){
            plr->grav = plr->grav_default;
            plr->state = Player_State::idle;
            plr->anim_index = 0;
        }
    }
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
