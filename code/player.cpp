/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */

#include "player.h"



void
player_create(Game_Pointers game_pointers, Vec2 _pos)
{
    Player* plr = &game_pointers.entity->player;
    
    plr->sprite = &game_pointers.data->sPlayer_idle;
    plr->anim_speed_mult = 0;
    
    plr->pos   = _pos;
    
    plr->size  = {4, Tile(1)-1};
    plr->scale = {1, 1};
    plr->color = GREEN;

    plr->state = Player_State::ground;
    plr->grav_default = 0.12f;
    plr->grav_low = 0.09f;
    plr->grav = plr->grav_default;
}

void
player_update(Game_Pointers game_pointers, Game_Input_Map input)
{
    Player* plr = &game_pointers.entity->player;
    
    float32 target_spd = 1.f;
    float32 jumpspd = 1.9f;
    
    Vec2 move_input = {(float32)(input.right.hold - input.left.hold),
                       (float32)(input.down.hold - input.up.hold)};

    
    plr->spd.x = approach(plr->spd.x, move_input.x * target_spd, 0.05f);

    if (plr->state == Player_State::ground)
    {
        //sprite
        if (plr->spd.x != 0){
            plr->sprite = &game_pointers.data->sPlayer_walk;
            plr->anim_speed_mult = abs_f32(plr->spd.x);
        }
        else{
            plr->anim_index = (float32)(input.up.hold);
            
            plr->sprite = &game_pointers.data->sPlayer_idle;
            plr->anim_speed_mult = 0;
        }

        //input
        if (move_input.x != 0)
            plr->scale.x = (float32)sign(move_input.x);
        
        if (input.jump){
            plr->spd.y -= jumpspd;
            plr->grav = plr->grav_low;
            plr->state = Player_State::air;            
        }

        //
        plr->spd.y += plr->grav;
        Collide_Data coll = move_collide_wall(game_pointers, &plr->pos, &plr->spd, plr->size);
    }
    else if (plr->state == Player_State::air)
    {
        plr->sprite = &game_pointers.data->sPlayer_walk;
        plr->anim_speed_mult = 0;
        plr->anim_index = 0;
        
        if (input.jump.release){
            if (plr->spd.y < 0)
            {
                plr->spd.y /= 3;
                plr->grav = plr->grav_default;
            }
        }
        plr->spd.y += plr->grav;
        Collide_Data coll = move_collide_wall(game_pointers, &plr->pos, &plr->spd, plr->size);
        if (coll.ydir == 1){
            plr->grav = plr->grav_default;
            plr->state = Player_State::ground;
            plr->anim_index = 0;
        }
    }
}

void
player_draw(Player* plr, Game_Pointers game_pointers)
{

    //DRAW BBOX
    // draw_rect(game_pointers, plr->pos, plr->size, color);
    
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
