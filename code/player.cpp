/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */

//TODO: move up
#define is_turning(_movex, _spdx) ((sign(_movex) != sign(_spdx) && _movex != 0))

void Entity::player_state_switch(Player_State new_state)
{
    player_state_perform(player_state, State_Function::Leave);
    player_state_perform(new_state, State_Function::Enter);
    player_state = new_state;
}

void
Entity::player_create(Vec2f _pos)
{
    *this = {};
    bbox = { .pos = {-2, -7}, .size = {4, Tile(1)-1} };
    sprite = &GSPRITE->sPlayer_idle;
    pos   = _pos;
    player_state_perform(player_state, State_Function::Enter);
}


void
Entity::player_update()
{
    auto input = GINPUT;
    move_input = {(f32)(input->right.hold - input->left.hold),
                  (f32)(input->down.hold - input->up.hold)};
    // IF_DEBUG { //HACK: overwrites our physics til we change state
    //     physics = debug_physics;
    // }

    player_state_perform(player_state, State_Function::Step);
    
    //collide with spike
    b32 spiked = collide_rect_entity(pos + bbox.pos, bbox.size, Ent_Type::Spike);
    if (spiked && !hurt_buffer_enabled && player_state != Player_State::Hurt)
        player_state_switch(Player_State::Hurt);

    //handle hurt buffer
    if (hurt_buffer_enabled){
        hurt_buffer_tick++;
        if (hurt_buffer_tick > hurt_buffer_time){
            hurt_buffer_enabled = false;
            hurt_buffer_tick = 0;
        }
    }    
}

// bool32
// Entity::anim_update()
// {
//     b32 result = false;
//     f32 index_add = (f32)sprite->fps/FPS_TARGET;
//     anim_index += index_add * anim_speed;

// //method1
//     anim_ended_this_frame = false;
//     if (anim_index + index_add >= sprite->frame_num) anim_ended_this_frame = true;
//     anim_index = mod_f32(anim_index, (f32)sprite->frame_num);
    
// //method2
//     // if (anim_index >= sprite->frame_num) {
//     //     anim_ended_this_frame = true;
//     //     result = true;
//     //     anim_index = 0;
//     // }
    
//     return result;
// }

void
Entity::player_draw()
{
//method1
    entity_anim_update(this); //NOTE: run before draw calls to keep index in bounds
    
    if (hurt_buffer_tick % 2 == 0)
        draw_sprite_frame(sprite, pos, anim_index, scale);
    
    IF_DEBUG{
        //draw ledge check pos
        draw_pixel(player_ledge_check_pos(), GREEN);

        //DRAW bbox & origin
        draw_rect(pos + bbox.pos, bbox.size, BBOXRED); //bbox
        draw_pixel(pos, WHITE); //origin

        draw_text_buffer(
            pos, {0.5, 0.5}, {5, 8},
            "state: %i" "\n"
            "x: %.2f"   "\n"
            "y: %.2f"   "\n"
            "xsp: %.2f" "\n"
            "ysp: %.2f" "\n"
            , player_state
            , x
            , y
            , spd.x
            , spd.y
        );
    }

//method2
    // anim_update();

}


void
Entity::player_speed_hori(bool32 is_airborne)
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
    if (is_moving) lerpfunc =        lerp;

    //apply speed
    spd.x = lerpfunc(spd.x, spd_target, accel_target);

    //reset physics
    physics = physsave;
}
void
Entity::player_speed_vert()
{
    spd.y += physics.grav;
    if (spd.y > terminal_velocity)
        spd.y = terminal_velocity;
}




//forward declaration
Collide_Data move_collide_tile(Tilemap* tmap, Vec2f* pos, Vec2f* spd, Vec2f size, Vec2f pos_offset = {}); //HACK:


V2f Entity::player_ledge_check_pos(i32* aimdir_return_var, i32 aimdir_override, f32 xmargin_override)
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
    if (xmargin_override != 0) xmargin = xmargin_override;
    result.x += xmargin * aimdir;
    if (aimdir_return_var) *aimdir_return_var = aimdir;
    return result;
}

//TODO: make sure there is space above the ledge point (threshold value?)
//NOTE: some hacky edgecases were handling to get the desired behavior:
//player can be max of 1 pixel away from the ledge to attach
bool32
Entity::player_ledge_check()
{
    b32 result = false;
    auto tmap = &GDATA->tilemap;
    b32 ledge_grab_either_side = true;
    i32 aimdir;
    V2f tilecheck_pos = player_ledge_check_pos(&aimdir);
    f32 xmargin_final = ledge_xmargin;
    b32 tile_at_corner = *tilemap_get_tile_at_world_pos(tmap, tilecheck_pos);
    if (!tile_at_corner && ledge_grab_either_side){ //grab either side regardless of aim
        xmargin_final = ledge_xmargin + 2;
        tilecheck_pos = player_ledge_check_pos(&aimdir, -sign(scale.x), xmargin_final);
        tile_at_corner = *tilemap_get_tile_at_world_pos(tmap, tilecheck_pos);
    }

    if (tile_at_corner){
        f32 tilerect_xoffset = aimdir > 0 ? 1.f : 0.f;
        Rect tilerect = tilemap_get_tile_rect(tmap, tilecheck_pos + V2f{tilerect_xoffset, 0});
        f32 tcorner_x = aimdir > 0 ? tilerect.x : tilerect.x + tmap->tile_w;
        V2f tile_corner = { tcorner_x, tilerect.y };
        
        f32 xdist = dist(tile_corner.x, tilecheck_pos.x); //distance to ledge
        f32 ydist = dist(tile_corner.y, tilecheck_pos.y); //
        b32 x_aligned = (xdist <= xmargin_final + (aimdir < 0));
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
Entity::player_state_perform(Player_State _state, State_Function _function)
{
    using enum Player_State;
    using enum State_Function;
    
    auto input = pointers->input;
    auto tmap = &pointers->data->tilemap;
    
    switch (_state){
      case Idle:{
          switch (_function){
            case Enter:{
                entity_state_enter_default(this, &GSPRITE->sPlayer_idle, 0, 0);
                physics = ground_physics;
            }break;

            case Step:{
                //anim
                anim_index = (float32)input->up.hold;
                //speed
                player_speed_vert();
                player_speed_hori(false);
                move_collide_tile(&GDATA->tilemap, &pos, &spd, bbox.size, bbox.pos);

                if (input->jump)
                    player_state_switch(Jump);
                else if (spd.y != 0)
                    player_state_switch(Fall);
                else if (move_input.x != 0 && spd.x != 0)
                    player_state_switch(Walk);
            }break;
          }
      }break;
            
      case Walk:{
          switch (_function){
            case Enter:{
                entity_state_enter_default(this, &GSPRITE->sPlayer_walk);
                physics = ground_physics;
            }break;

            case Step:{
                f32 slow_physics_spd_threshold = 0.45f;
                
                player_speed_hori(false);
                player_speed_vert();
                move_collide_tile(&GDATA->tilemap, &pos, &spd, bbox.size, bbox.pos);

                //anim
                if (is_turning(move_input.x, spd.x))
                    sprite = &GSPRITE->sPlayer_turn;
                else if (input->up.hold)
                    sprite = &GSPRITE->sPlayer_walk_reach;
                else
                    sprite = &GSPRITE->sPlayer_walk;

                if (move_input.x != 0)
                    scale.x = (float32)sign(move_input.x);
                anim_speed = abs_f32(spd.x);

                //state
                if (input->jump)
                    player_state_switch(Jump);
                else if (spd.y != 0){
                    player_state_switch(Fall);
                    coyote_timer.start();
                    
                    if (abs_f32(spd.x) < slow_physics_spd_threshold)
                        if (!input->shift.hold) //TODO: sprint input
                            physics = fall_physics_slow;
                }else if (spd.x == 0)
                    player_state_switch(Idle);
            }break;
          }
      }break;

//Air
      case Jump:{
          switch (_function){
            case Enter:{
                entity_state_enter_default(this, &GSPRITE->sPlayer_air, 0, 0);
                physics = jump_physics;
                spd.y = -jump_spd;
            }break;

            case Step:{
                f32 yspd_threshold = 0.4f;
                b32 reach_time = 4;

                state_timer++;
                if (abs_f32(spd.y) > yspd_threshold){
                    if (input->up.hold && state_timer >= reach_time){
                        sprite = &GSPRITE->sPlayer_air_reach;
                        anim_index = (move_input.x != 0 && move_input.x != scale.x);
                    }else{
                        sprite = &GSPRITE->sPlayer_air;
                    }
                }else{
                    anim_speed = abs_f32(spd.y) * 1.6f;
                }

                //speed
                player_speed_vert();
                player_speed_hori(true);
                if (!input->jump.hold) spd.y *= 0.9f;
                Collide_Data coll = move_collide_tile( &GDATA->tilemap, &pos, &spd, bbox.size, bbox.pos);

                //state
                if (move_input.y < 0 && player_ledge_check())
                    player_state_switch(Ledge);
                else if (spd.y > 0)
                    player_state_switch(Fall);

            }break;
          }
      }break;

      case Fall:{
          switch (_function){
            case Enter:{
                Sprite* spr = input->up.hold ? &GSPRITE->sPlayer_air_reach : &GSPRITE->sPlayer_air;
                float32 index = input->up.hold ? anim_index : 0;
                entity_state_enter_default(this, spr, index, 0);
                
                physics = fall_physics;
            }break;
            
            case Step:{
                coyote_timer.update();

                if (input->space)
                    if (!roll_buffer_timer.is_active)
                        roll_buffer_timer.start();
                roll_buffer_timer.update();
                
                //anim
                if (input->up.hold){
                    sprite = &GSPRITE->sPlayer_air_reach;
                    anim_index = (move_input.x != 0 && move_input.x != scale.x);
                }else{
                    sprite = &GSPRITE->sPlayer_air;
                }
                anim_speed = abs_f32(spd.y) * 0.8f;
                
                //movecollide
                if (input->shift.press){
                    f32 spd_add = 0.25f;
                    if (abs_f32(spd.x) < physics.maxspd - spd_add)
                        spd.x += (spd_add * sign(spd.x));
                    physics = fall_physics;
                }                
                player_speed_hori(true);
                player_speed_vert();
                Collide_Data coll = move_collide_tile(&pointers->data->tilemap, &pos, &spd, bbox.size, bbox.pos);
                b32 hit_ground = (coll.ydir == 1);

                
                //states
                if (coyote_timer.is_active && input->jump)
                    player_state_switch(Jump);
                else if (input->up.hold && player_ledge_check())
                    player_state_switch(Ledge);
                else if (hit_ground)
                {
                    auto ground_st = (spd.x != 0 ? Walk : Idle);
                    f32 yspd_highland_threshold = 3.2f;
                    
                    if (coll.spd_save.y >= yspd_highland_threshold){
                        b32 roll_buffered = roll_buffer_timer.is_active;
                        if (roll_buffered) ground_st = Roll;
                        else ground_st = Splat;
                    }
                    player_state_switch(ground_st);
                }
            }break;
          }
      }break;
      
      case Ledge:{ 
          switch (_function){
            case Enter:{
                Sprite* spr = (spd.y > 0 ? &GSPRITE->sPlayer_ledge_grab : &GSPRITE->sPlayer_ledge);
                entity_state_enter_default(this, spr);
                spd = {};
            }break;
              
            case Step:{
                state_timer++;

                x = lerp(x, ledge_xtarget, 0.32f);
                
                f32 ledgegrab_anim_time = (spd.y*2);
                if (state_timer >= ledgegrab_anim_time)
                    sprite = &GSPRITE->sPlayer_ledge;

                f32 time_to_reach_up = 5 + ledgegrab_anim_time;
                b32 can_reach = state_timer >= time_to_reach_up;

                f32 movex = move_input.x;
                b32 trying2aim = move_input.y != 0 || (movex != 0 && movex != scale.x);
                b32 reaching_up = move_input.y > 0;
                b32 reaching_down = move_input.y > 0;
                b32 reaching_away = sign(movex) == -sign(scale.x);
                
                if (trying2aim){
                    if (can_reach){
                        //NOTE: ledge aim sprite:
                        sprite = &GSPRITE->sPlayer_ledge_reach;
                        if (reaching_down)         anim_index = 2;
                        else if (reaching_away)    anim_index = 1;
                        else if (move_input.y < 0) anim_index = 0;
                    }
                }else{
                    sprite = &GSPRITE->sPlayer_ledge;
                }
                
                //state
                if (input->jump){
                    player_state_switch(Jump);
                    if (reaching_down)
                        spd.y = 0.1f;
                    else if (reaching_away){
                        spd.x = movex * 0.85f;
                        spd.y = -1.1f;
                    }
                    
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
                entity_state_enter_default(this, &GSPRITE->sPlayer_hurt);
            }break;
              
            case Step:{
                state_timer++;
                if (state_timer > hurt_state_time)
                    player_state_switch(Idle);
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

      case Splat:{
          switch (_function){
            case Enter:{
                entity_state_enter_default(this, &GSPRITE->sPlayer_splat_swift, 0, 0);
            }break;
              
            case Step:{
                f32 lerp_spd = 0.04;
                spd.x = approach(spd.x, 0, lerp_spd);
                player_speed_vert();
                Collide_Data coll = move_collide_tile(&pointers->data->tilemap, &pos, &spd, bbox.size, bbox.pos);

                if (input->jump.press)
                    if (spd.x == 0 && spd.y == 0)
                        anim_speed = 1;
                
                if (anim_ended_this_frame)
                    player_state_switch(Idle);
            }break;
          }
      }break;

      case Roll:{
          switch (_function){
            case Enter:{
                entity_state_enter_default(this, &GSPRITE->sPlayer_roll);
                scale.x = (f32)sign(spd.x);
                f32 roll_remaining = roll_buffer_timer.remaining();
                f32 roll_norm = (f32)roll_remaining / roll_buffer_timer.length;
                f32 roll_spd_target = roll_norm * 1.25f;
                spd.x = roll_spd_target * (f32)sign(scale.x);
            }break;

            case Step:{
                if (input->jump.press)
                    anim_speed = 1;

                // f32 lerp_spd = f32(state_timer++ / 10);
                // spd.x = approach(spd.x, 0, 0.01f);
                player_speed_vert();
                Collide_Data coll = move_collide_tile(&pointers->data->tilemap, &pos, &spd, bbox.size, bbox.pos);

                if (anim_ended_this_frame)
                    player_state_switch(Idle);
            }break;
          }
      }break;
        
      default: {
          int neat = true;
      };
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
    //     player_speed_hori(plr, true);
    //     if (!input->jump.hold){
    //         if (plr->spd.y < 0)
    //             plr->spd.y /= 4;
    //     }
    // }else if (plr->spd.x != 0) //walk
    // {
    //     if (input->jump) plr->spd.y = -plr->jump_spd;
    //     sprite_change(plr, sPlayer_walk);
    //     player_speed_hori(plr, false);
    //     if (plr->move_input.x != 0) plr->scale.x = (float32)sign(plr->move_input.x);
    //     plr->anim_speed = abs_f32(plr->spd.x);
    // }
    // else{ //idle
    //     if (input->jump) plr->spd.y = -plr->jump_spd;
    //     sprite_change(plr, sPlayer_idle);
    //     plr->anim_index = (float32)input->up.hold;
    //     player_speed_hori(plr, false);
    // }



/* DRAW SMILEY FACE
  
    float32 smile_size = 4;
    draw_rect({pos.x, pos.y}, {smile_size, smile_size * 5}, blue);
    draw_rect({pos.x - smile_size, pos.y-smile_size}, {smile_size, smile_size}, red);
    draw_rect({pos.x + smile_size*5, pos.y-smile_size}, {smile_size, smile_size}, red);
    draw_rect({pos.x, pos.y - (smile_size*3)}, {smile_size, smile_size}, red);
    draw_rect({pos.x + (smile_size*4), (pos.y-smile_size*3)}, {smile_size, smile_size}, red);

 */
