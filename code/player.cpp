/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */

//TODO: move up
// #define PLAYER_LEDGE_GRAB_SPACE_MIN 1 //pixels
#define is_turning(movex, spdx) ((sign(movex) != sign(spdx) && movex != 0))


void Player::state_enter_default(Sprite* _sprite, float32 _anim_index, float32 _anim_speed)
{
    sprite = _sprite;
    anim_index = _anim_index;
    anim_speed = _anim_speed;
    state_timer = 0;
}
void Player::state_switch(Player_State new_state)
{
    state_perform(state, State_Function::Leave);
    state_perform(new_state, State_Function::Enter);
    state = new_state;
}


void
Player::Create(Vec2f _pos)
{
    *this = {};
    sprite = &GSPRITE->sPlayer_idle;
    pos   = _pos;
    state_perform(state, State_Function::Create);
}


void
Player::Update(Game_Input_Map* input)
{
    move_input = {(float32)(input->right.hold - input->left.hold),
                  (float32)(input->down.hold - input->up.hold)};
    // IF_DEBUG { //HACK: overwrites our physics til we change state
    //     physics = debug_physics;
    // }

    state_perform(state, State_Function::Step);
    
    //collide with spike
    b32 spiked = collide_rect_entity(pos + bbox.pos, bbox.size, Ent_Type::Spike);
    if (spiked && !hurt_buffer_enabled && state != Player_State::Hurt)
        state_switch(Player_State::Hurt);

    //handle hurt buffer
    if (hurt_buffer_enabled){
        hurt_buffer_tick++;
        if (hurt_buffer_tick > hurt_buffer_time){
            hurt_buffer_enabled = false;
            hurt_buffer_tick = 0;
        }
    }
    
    // IF_DEBUG {
    //     if (GINPUT->jump)
    //         spd.y = -2.5;
    // }
}


void
Player::Draw()
{
    //NOTE: should we do this afterwards? If we dont then anim_index will never be 0 unless the math happens to work out between sprite fps and anim_speed
    anim_index += ((float32)sprite->fps/FPS_TARGET) * anim_speed;
    if (anim_index >= sprite->frame_num) anim_index = 0;

    if (hurt_buffer_tick % 2 == 0)
    draw_sprite_frame(sprite, pos, anim_index, scale);

    IF_DEBUG{
        //draw ledge check pos
        draw_pixel(ledge_check_pos(), GREEN);

        //DRAW bbox & origin
        draw_rect(pos + bbox.pos, bbox.size, BBOXRED); //bbox
        draw_pixel(pos, WHITE); //origin

        draw_text_buffer(pos, {0.5, 0.5}, {5, 8},
                         "state: %i" "\n"
                         "pos:\nx:%.2f\ny:%.2f" "\n"
                         , state
                         , x
                         , y
        );
    }
}


void
Player::move_hori(bool32 is_airborne)
{
    //TODO: test out 'true' lerp with counter and all that ON Physics::accel
    
    //NOTE: sprinting acceleration test
    auto physsave = physics;
    if (GINPUT->shift.hold
    && !is_airborne)
        physics.accel *= 1.85f;

    //determine spd/accel
    f32 spd_target = move_input.x * physics.maxspd;
    f32 accel_target = physics.decel;
    b32 is_moving = (move_input.x != 0);
    b32 plr_is_turning = is_turning(move_input.x, spd.x);
    if (is_moving)
        accel_target = physics.accel;
    if (plr_is_turning)
        accel_target = physics.turn;

    //determine func
    f32 (*lerpfunc)(f32, f32, f32) = approach;
    if (is_moving) lerpfunc = lerp;

    //apply speed
    spd.x = lerpfunc(spd.x, spd_target, accel_target);

    //reset physics
    physics = physsave;
}
void
Player::move_vert()
{
    spd.y += physics.grav;
    if (spd.y > terminal_velocity)
        spd.y = terminal_velocity;
}




//forward declaration
Collide_Data move_collide_tile(Tilemap* tmap, Vec2f* pos, Vec2f* spd, Vec2f size, Vec2f pos_offset = {}); //HACK:


V2f Player::ledge_check_pos(i32* aimdir_return_var, i32 aimdir_override, f32 ledge_xmargin_override)
{    
    i32 aimdir = sign(move_input.x);
    if (aimdir == 0) aimdir = sign(spd.x);
    if (aimdir == 0) aimdir = sign(scale.x);
    if (aimdir_override != 0) aimdir = aimdir_override;

    V2f result = {
        aimdir < 0 ? bbox_left(this) : bbox_right(this),
        bbox_top(this) - 1
    };
    f32 xmargin = ledge_xmargin;
    if (ledge_xmargin_override != 0) xmargin = ledge_xmargin_override;
    result.x += xmargin * aimdir;
    if (aimdir_return_var) *aimdir_return_var = aimdir;
    return result;
}

//TODO: make sure there is space above the ledge point (threshold value?)
//NOTE: some hacky edgecases were handling to get the desired behavior:
//player can be max of 1 pixel away from the ledge to attach
bool32
Player::ledge_grab()
{
    b32 result = false;
    auto tmap = &GDATA->tilemap;

    b32 ledge_grab_either_side = true;

    i32 aimdir;
    V2f tilecheck_pos = ledge_check_pos(&aimdir);
    // b32 moving_away_from_aimdir = (sign(spd.x) == -sign(aimdir));
    // if (moving_away_from_aimdir)
        // tilecheck_pos = ledge_check_pos(&aimdir);
    
    b32 tile_at_corner = *tilemap_get_tile_at_world_pos(tmap, tilecheck_pos);
    if (!tile_at_corner && ledge_grab_either_side){ //grab either side regardless of aim
        tilecheck_pos = ledge_check_pos(&aimdir, -sign(scale.x));
        tile_at_corner = *tilemap_get_tile_at_world_pos(tmap, tilecheck_pos);
    }

    if (tile_at_corner){
        f32 tilerect_xoffset = aimdir > 0 ? 1.f : 0.f;
        Rect tilerect = tilemap_get_tile_rect(tmap, tilecheck_pos + V2f{tilerect_xoffset, 0});
        f32 tcorner_x = aimdir > 0 ? tilerect.x : tilerect.x + tmap->tile_w;
        V2f tile_corner = { tcorner_x, tilerect.y };
        
        f32 xdist = dist(tile_corner.x, tilecheck_pos.x);
        f32 ydist = dist(tile_corner.y, tilecheck_pos.y);
        b32 x_aligned = (xdist <= ledge_xmargin + (aimdir < 0));
        b32 y_aligned = (ydist <= abs_f32(spd.y));
        b32 corners_aligned = x_aligned && y_aligned;
        b32 tile_above_is_open = !collide_rect_tilemap(tile_corner + V2f{(f32)aimdir, -1}, {1, 1}, tmap);
        
        if (corners_aligned && tile_above_is_open){
            if (aimdir > 0)
                ledge_xtarget = tile_corner.x - bbox.w/2;
            if (aimdir < 0)
                ledge_xtarget = tile_corner.x + bbox.w/2;
            
            y = tile_corner.y + bbox.h + 1;
            scale.x = (f32)sign(aimdir);
            result = true;
        }
    }

    return result;
}


void
Player::state_perform(Player_State _state, State_Function _function)
{
    using enum Player_State;
    using enum State_Function;
    
    auto input = pointers->input;
    auto tmap = &pointers->data->tilemap;
    
    switch (_state)
    {
      case Idle:{
          switch (_function){   
            case Enter:{
                state_enter_default(&GSPRITE->sPlayer_idle, 0, 0);
                physics = ground_physics;
            }break;

            case Step:{
                //anim
                anim_index = (float32)input->up.hold;
                //speed
                move_vert();
                move_hori(false);
                move_collide_tile(&GDATA->tilemap, &pos, &spd, bbox.size, bbox.pos);

                if (input->jump)
                    state_switch(Jump);
                else if (spd.y != 0)
                    state_switch(Fall);
                else if (move_input.x != 0 && spd.x != 0)
                    state_switch(Walk);
            }break;
          }
      }break;
            
      case Walk:{
          switch (_function){
            case Enter:{
                state_enter_default(&GSPRITE->sPlayer_walk);
                physics = ground_physics;
            }break;

            case Step:{
                move_vert();
                move_hori(false);
                move_collide_tile(&GDATA->tilemap, &pos, &spd, bbox.size, bbox.pos);

                //anim
                if (is_turning(move_input.x, spd.x))
                    sprite = &GSPRITE->sPlayer_turn;
                else if (input->up.hold)
                    sprite = sprite = &GSPRITE->sPlayer_walk_reach;
                else
                    sprite = sprite = &GSPRITE->sPlayer_walk;

                if (move_input.x != 0)
                    scale.x = (float32)sign(move_input.x);
                anim_speed = abs_f32(spd.x);

                //state
                if (input->jump)
                    state_switch(Jump);
                else if (spd.y != 0){
                    state_switch(Fall);
                    if (spd.x < 0.35f && !input->shift.hold) physics = fall_physics_slow;
                }else if (spd.x == 0)
                    state_switch(Idle);
            }break;
          }
      }break;

//Air
      case Jump:{
          switch (_function){
            case Enter:{
                state_enter_default(&GSPRITE->sPlayer_air, 0, 0);
                physics = jump_physics;
                spd.y = -jump_spd;
            }break;

            case Step:{
                state_timer++;
                //anim
                f32 yspd_threshold = 0.4f;
                b32 adsasd = state_timer > 4;

                if (abs_f32(spd.y) > yspd_threshold){
                    if (input->up.hold && adsasd){
                        sprite = &GSPRITE->sPlayer_air_reach;
                        anim_index = (move_input.x != 0 && move_input.x != scale.x);
                    }else{
                        sprite = &GSPRITE->sPlayer_air;
                    }
                }else{
                    anim_speed = abs_f32(spd.y) * 1.6f;
                }

                //speed
                move_vert();
                move_hori(true);
                if (!input->jump.hold) spd.y *= 0.9f;
                Collide_Data coll = move_collide_tile( &GDATA->tilemap, &pos, &spd, bbox.size, bbox.pos);

                //state
                if (input->up.hold && ledge_grab())
                    state_switch(Ledge);
                else if (spd.y > 0)
                    state_switch(Fall);

            }break;
          }
      }break;

      case Fall:{
          switch (_function){
            case Enter:{
                Sprite* spr = pointers->input->up.hold ? &GSPRITE->sPlayer_air_reach : &GSPRITE->sPlayer_air;
                float32 index = pointers->input->up.hold ? anim_index : 0;
                state_enter_default(spr, index, 0);
        
                physics = fall_physics;
            }break;
            
            case Step:{
                //anim
                if (input->up.hold){
                    sprite = &GSPRITE->sPlayer_air_reach;
                    anim_index = (move_input.x != 0 && move_input.x != scale.x);
                }else{
                    sprite = &GSPRITE->sPlayer_air;
                }
                anim_speed = abs_f32(spd.y) * 0.8f;
        
                move_hori(true);
                move_vert();
                Collide_Data coll = move_collide_tile(&pointers->data->tilemap, &pos, &spd, bbox.size, bbox.pos);
                b32 hit_ground = coll.ydir == 1;
                
                if (input->up.hold && ledge_grab())
                    state_switch(Ledge);
                else if (hit_ground){ //land
                    //TODO: more landing states
                    // if ()
                    auto ground_st = (spd.x != 0 ? Walk : Idle);
                    state_switch(ground_st);
                }
            }break;
          }
      }break;
      
      case Ledge:{ 
          switch (_function){
            case Enter:{
                state_enter_default(&GSPRITE->sPlayer_ledge_grab);
            }break;
              
            case Step:{
                state_timer++;

                x = lerp(x, ledge_xtarget, 0.25f);
                
                f32 ledgegrab_anim_time = (spd.y*2);
                if (state_timer >= ledgegrab_anim_time)
                    sprite = &GSPRITE->sPlayer_ledge;

                f32 movex = move_input.x;
                f32 time_to_reach_up = 5 + ledgegrab_anim_time;
                b32 trying2aim = move_input.y != 0 || (movex != 0 && movex != scale.x);
                b32 can_reach = state_timer >= time_to_reach_up;
                if (trying2aim){
                    if (can_reach){
                        sprite = &GSPRITE->sPlayer_ledge_reach;
                        if (move_input.y > 0) anim_index = 2;
                        else if (sign(movex) == -sign(scale.x)) anim_index = 1;
                        else if (input->up.hold) anim_index = 0;
                    }
                }else{
                    sprite = &GSPRITE->sPlayer_ledge;
                }
                
                //state
                if (input->jump){
                    state_switch(Jump);
                    if (move_input.y > 0) spd.y = 0.1f;
                    else if (move_input.x) spd.x = move_input.x * 0.85f;
                    if (spd.x != 0)
                        scale.x = (f32)sign(spd.x);
                }
            }break;
              
            case Leave:{
            }break;
          }
      }break;

      case Hurt:{
          switch (_function){
            case Enter:{
                state_enter_default(&GSPRITE->sPlayer_hurt);
            }break;
              
            case Step:{
                state_timer++;
                if (state_timer > hurt_state_time)
                    state_switch(Idle);
            }break;
              
            case Leave:{
                hurt_buffer_tick = 0;
                hurt_buffer_enabled = true;

                b32 turning = is_turning(move_input.x, spd.x);
                if (turning || move_input.x == 0) spd.x /= 3;
                spd.y = 0;
            }break;
          }
      }break;
    }
}
//STATE TEMPLATE
// case Player_State::Jump:
// {
//     switch (_function){
//       case Enter:{
//       }break;
//
//       case Step:{
//       }break;
//
//       case Leave:{
//       }break;
//     }
// }break;



//proto state
    // if (plr->spd.y != 0) //air
    // {
    //     sprite_change(plr, sPlayer_air);
    //     player_move_hori(plr, true);
    //     if (!input->jump.hold){
    //         if (plr->spd.y < 0)
    //             plr->spd.y /= 4;
    //     }
    // }else if (plr->spd.x != 0) //walk
    // {
    //     if (input->jump) plr->spd.y = -plr->jump_spd;
    //     sprite_change(plr, sPlayer_walk);
    //     player_move_hori(plr, false);
    //     if (plr->move_input.x != 0) plr->scale.x = (float32)sign(plr->move_input.x);
    //     plr->anim_speed = abs_f32(plr->spd.x);
    // }
    // else{ //idle
    //     if (input->jump) plr->spd.y = -plr->jump_spd;
    //     sprite_change(plr, sPlayer_idle);
    //     plr->anim_index = (float32)input->up.hold;
    //     player_move_hori(plr, false);
    // }



/* DRAW SMILEY FACE
  
    float32 smile_size = 4;
    draw_rect({pos.x, pos.y}, {smile_size, smile_size * 5}, blue);
    draw_rect({pos.x - smile_size, pos.y-smile_size}, {smile_size, smile_size}, red);
    draw_rect({pos.x + smile_size*5, pos.y-smile_size}, {smile_size, smile_size}, red);
    draw_rect({pos.x, pos.y - (smile_size*3)}, {smile_size, smile_size}, red);
    draw_rect({pos.x + (smile_size*4), (pos.y-smile_size*3)}, {smile_size, smile_size}, red);

 */
