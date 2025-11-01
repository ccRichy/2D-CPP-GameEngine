/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */

void state_enter_default(
    Player* plr, Sprite* sprite, float32 anim_index = 0, float32 anim_speed = 1)
{
    plr->sprite = sprite;
    plr->anim_index = anim_index;
    plr->anim_speed = anim_speed;
}


void
player_move_hori(Player* plr, bool32 is_airborne)
{
    float32 spd_target = plr->move_input.x * plr->physics.maxspd;
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
player_move_vert(Player* plr)
{
    plr->spd.y += plr->physics.grav;
    if (plr->spd.y > plr->terminal_velocity)
        plr->spd.y = plr->terminal_velocity;
}






void Player::state_switch(Player_State new_state)
{
    state_perform(state, State_Function::Leave);
    state_perform(new_state, State_Function::Enter);
    state = new_state;
}


//forward declaration for following functions
Collide_Data move_collide_tile(Tilemap* tmap, Vec2f* pos, Vec2f* spd, Vec2f size); //HACK:

void
Player::state_perform(Player_State _state, State_Function _function)
{
    using enum Player_State;
    using enum State_Function;
    
    auto input = pointers->input;
    auto tmap = &pointers->data->tilemap;
    
    switch (_state){
      case Idle:{
          
          switch (_function){   
            case Enter:{
                state_enter_default(this, &GSPRITE->sPlayer_idle, 0, 0);
                physics = ground_physics;
            }break;
              
            case Step:{
                //anim
                anim_index = (float32)input->up.hold;
                //speed
                player_move_vert(this);
                player_move_hori(this, false);
                move_collide_tile(&GDATA->tilemap, &pos, &spd, size);

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
                state_enter_default(this, &GSPRITE->sPlayer_walk);
                physics = ground_physics;
            }break;
              
            case Step:{
                spd.y += physics.grav;
                player_move_hori(this, false);
                move_collide_tile(&GDATA->tilemap, &pos, &spd, size);

                //anim
                if (sign(move_input.x) != sign(spd.x) && move_input.x != 0)
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
                else if (spd.y != 0)
                    state_switch(Fall);
                else if (spd.x == 0)
                    state_switch(Idle);
            }break;
          }
      }break;

//Air
      case Jump:{
          switch (_function){
            case Enter:{
                state_enter_default(this, &GSPRITE->sPlayer_air, 0, 0);
                physics = jump_physics;
                spd.y = -jump_spd;
            }break;

            case Step:{
                //anim
                float32 yspd_threshold = 0.4f;
                if (abs_f32(spd.y) > yspd_threshold){
                    if (input->up.hold){
                        sprite = &GSPRITE->sPlayer_air_reach;
                        anim_index = (move_input.x != 0 && move_input.x != scale.x);
                    }else{
                        sprite = &GSPRITE->sPlayer_air;
                    }
                }else{
                    anim_speed = abs_f32(spd.y) * 1.6f;
                }

                //speed
                player_move_vert(this);
                player_move_hori(this, true);
                if (!input->jump.hold) spd.y *= 0.9f;
                Collide_Data coll = move_collide_tile( &GDATA->tilemap, &pos, &spd, size );

                //try to hard code ledge grabbing:
                //step 1a: find the tile where we are aiming
                i32 aimdir = sign(move_input.x);
                // if (aimdir == 1)
                //step 1b determine which tile we are trying to ledge grab with
                V2f worldpos_to_check = {};
                V2i tile_pos_at_ledge = tilemap_get_grid_pos(tmap, worldpos_to_check);
                
                // if (){
                    
                // }

                //state
                if (spd.y > 0)
                    state_switch(Fall);

            }break;
          }
      }break;

      case Fall:{
          switch (_function){
            case Enter:{
                Sprite* spr = pointers->input->up.hold ? &GSPRITE->sPlayer_air_reach : &GSPRITE->sPlayer_air;
                float32 index = pointers->input->up.hold ? anim_index : 0;
                state_enter_default(this, spr, index, 0);
        
                physics = fall_physics;
            }
            break;
            
            case Step:{
                //anim
                if (input->up.hold){
                    sprite = &GSPRITE->sPlayer_air_reach;
                    anim_index = (move_input.x != 0 && move_input.x != scale.x);
                }else{
                    sprite = &GSPRITE->sPlayer_air;
                }
                anim_speed = abs_f32(spd.y) * 0.8f;
        
                player_move_hori(this, true);
                player_move_vert(this);
                Collide_Data coll = move_collide_tile(&pointers->data->tilemap, &pos, &spd, size);

                auto st = (spd.x != 0 ? Walk : Idle);
                if (coll.ydir == 1)
                    state_switch(st);
            }break;
          }
      }break;
      
      case Ledge:
      {
          switch (_function){
            case Enter:{
                sprite_set(this, sPlayer_ledge);
            }break;
              
            case Step:{
            }break;
              
            case Leave:{
            }break;
          }
      }break;

      case Hurt:
      {
          switch (_function){
            case Enter:{
                sprite_set(this, sPlayer_hurt);
            }break;
              
            case Step:{
                if (input->jump) state_switch(Idle);
            }break;
              
            case Leave:{
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



void
Player::Create(Vec2f _pos)
{
    // *this = Player;
    Player plr = {};
    plr.sprite = &GSPRITE->sPlayer_idle;
    state_perform(plr.state, State_Function::Create);
    //
    plr.pos   = _pos;
    plr.physics = plr.ground_physics;
    // plr.grav = plr.grav_default;

    *this = plr;

// #if 0
    // plr->st_idle.Enter = player_idle_enter;
    // plr->st_idle.Step = player_idle_step;
    // plr->st_walk.Enter = player_walk_enter;
    // plr->st_walk.Step = player_walk_step;
    // plr->st_jump.Enter = player_jump_enter;
    // plr->st_jump.Step = player_jump_step;
    // plr->st_fall.Enter = player_fall_enter;
    // plr->st_fall.Step = player_fall_step;
    
    // plr->state = &plr->st_idle;
    // plr->state->Enter();
//#endif
}



void
Player::Update(Game_Input_Map* input)
{
    move_input = {(float32)(input->right.hold - input->left.hold),
                  (float32)(input->down.hold - input->up.hold)};


    debug_physics =   {1, 0.1f,  0.1f,   0.1f,  0.07f};
    Physics phys_save = physics;
    IF_DEBUG physics = debug_physics;

    state_perform(state, State_Function::Step);

    //collide with spike
    b32 spiked = collide_rect_entity(pos, size, Ent_Type::Spike);
    if (spiked) state_switch(Player_State::Hurt);

    
    IF_DEBUG {
        physics = phys_save;
        if (GINPUT->jump)
            spd.y = -3;
    }
    
//movecollide
    // spd.y += physics.grav;
    //  if (spd.y > terminal_velocity)
    //     spd.y = terminal_velocity;

    // Collide_Data coll = move_collide_tile(&pointers->data->tilemap, &pos, &spd, size);
    // Collide_Data coll = move_collide_wall(&plr->pos, &plr->spd, plr->size);

//proto sprint
    // if (input->shift.hold) ground_speed_max = 2;
    // else ground_speed_max = 1;
}

void
Player::Draw()
{
    anim_index += ((float32)sprite->fps/FPS_TARGET) * anim_speed;
    if (anim_index >= sprite->frame_num) anim_index = 0;


    draw_sprite_frame(sprite, pos, anim_index, scale);

    //NOTE: should we do this afterwards? If we dont then anim_index will never be 0 unless the math happens to work out between sprite fps and anim_speed

    //DRAW bbox & origin
    // draw_rect(pos, size, RED);    
    // draw_pixel(pos, WHITE);


    IF_DEBUG{
        draw_text_buffer(pos + V2f{0, 10}, {0.5, 0.5}, {5, 8},
                         "state: %i" "\n"
                         , state
        );
    }
}


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
