/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */

//TODO: add enemy and wall
Sprite* entity_sprite_default(Ent_Type type)
{
    Sprite* result = 0;
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
player_move_hori(Player* plr, bool32 is_airborne)
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



// #if 0
/////STATES
void //WARNING: deprecated
state_change(State** state_variable, State* target)
{
    *state_variable = target;
    target->Enter();
}
void state_enter_default(Player* plr, Sprite* sprite, float32 anim_index = 0, float32 anim_speed = 1)
{
    plr->sprite = sprite;
    plr->anim_index = anim_index;
    plr->anim_speed = anim_speed;
}


//IDLE
void player_idle_enter()
{
    Player* plr = PLAYER;
    state_enter_default(plr, &GSPRITE->sPlayer_idle, 0, 0);
    plr->physics = plr->ground_physics;
}
void
player_idle_step(Game_Input_Map input)
{
    Player* plr = PLAYER;
    //anim
    plr->anim_index = (float32)input.up.hold;
    //speed
    player_gravity(plr);
    player_move_hori(plr, false);
    move_collide_wall(&plr->pos, &plr->spd, plr->size);
        
    // if (input.jump)
    //     state_change(&plr->state, &plr->st_jump);
    // else if (plr->spd.y != 0)
    //     state_change(&plr->state, &plr->st_fall);
    // else if (plr->move_input.x != 0 && plr->spd.x != 0)
    //     state_change(&plr->state, &plr->st_walk);
}




//WALK
void
player_walk_enter()
{
    Player* plr = PLAYER;
    state_enter_default(plr, &GSPRITE->sPlayer_walk);
    plr->physics = plr->ground_physics;
}
void
player_walk_step(Game_Input_Map input)
{
    Player* plr = PLAYER;

    plr->spd.y += plr->physics.grav;
    player_move_hori(plr, false);
    move_collide_wall(&plr->pos, &plr->spd, plr->size);

    //anim
    if (sign(plr->move_input.x) != sign(plr->spd.x) && plr->move_input.x != 0)
        plr->sprite = &GSPRITE->sPlayer_turn;
    else if (input.up.hold)
        plr->sprite = plr->sprite = &GSPRITE->sPlayer_walk_reach;
    else
        plr->sprite = plr->sprite = &GSPRITE->sPlayer_walk;

    if (plr->move_input.x != 0)
        plr->scale.x = (float32)sign(plr->move_input.x);
    plr->anim_speed = abs_f32(plr->spd.x);

    //state
    // if (input.jump)
    //     state_change(&plr->state, &plr->st_jump);
    // else if (plr->spd.y != 0)
    //     state_change(&plr->state, &plr->st_fall);
    // else if (plr->spd.x == 0)
    //     state_change(&plr->state, &plr->st_idle);
}

//JUMP
void
player_jump_enter()
{
    Player* plr = PLAYER;
    state_enter_default(plr, &GSPRITE->sPlayer_air, 0, 0);

    plr->physics = plr->jump_physics;
    plr->spd.y = -plr->jump_spd;
    // plr->grav = plr->grav_low;
}
void
player_jump_step(Game_Input_Map input)
{
    Player* plr = PLAYER;

    //anim
    float32 yspd_threshold = 0.4f;
    if (abs_f32(plr->spd.y) > yspd_threshold){
        if (input.up.hold){
            plr->sprite = &GSPRITE->sPlayer_air_reach;
            plr->anim_index = (plr->move_input.x != 0 && plr->move_input.x != plr->scale.x);
        }else{
            plr->sprite = &GSPRITE->sPlayer_air;
        }
    }else{
        plr->anim_speed = abs_f32(plr->spd.y) * 1.6f;
    }
        
    //speed
    player_gravity(plr);
    player_move_hori(plr, true);
    if (!input.jump.hold) plr->spd.y *= 0.9f;
    Collide_Data coll = move_collide_wall(&plr->pos, &plr->spd, plr->size);

    //state
    // if (plr->spd.y > 0)
    //     state_change(&plr->state, &plr->st_fall);
}
    
//FALL
void
player_fall_enter()
{
    Player* plr = PLAYER;
    Sprite* spr = pointers->input->up.hold ? &GSPRITE->sPlayer_air_reach : &GSPRITE->sPlayer_air;
    float32 index = pointers->input->up.hold ? plr->anim_index : 0;
    state_enter_default(plr, spr, index, 0);
        
    plr->physics = plr->fall_physics;
}
void
player_fall_step(Game_Input_Map input)
{
    Player* plr = PLAYER;

    //anim
    if (input.up.hold){
        plr->sprite = &GSPRITE->sPlayer_air_reach;
        plr->anim_index = (plr->move_input.x != 0 && plr->move_input.x != plr->scale.x);
    }else{
        plr->sprite = &GSPRITE->sPlayer_air;
    }
    plr->anim_speed = abs_f32(plr->spd.y) * 0.8f;

        
    player_move_hori(plr, true);
    player_gravity(plr);
    Collide_Data coll = move_collide_wall(&plr->pos, &plr->spd, plr->size);
        
    // if (coll.ydir == 1)
    //     state_change(&plr->state,
    //                  plr->spd.x != 0 ? &plr->st_walk : &plr->st_idle);
};
// #endif




//TODO: move theez
#define sprite_change(__entity, __sprite) __entity->sprite = &pointers->sprite->##__sprite
Collide_Data move_collide_tile(Tilemap* tmap, Vec2f* pos, Vec2f* spd, Vec2f size);




void Player::state_switch(Player_State new_state)
{
    state_perform(state, State_Function::Leave);
    state_perform(new_state, State_Function::Enter);
    state = new_state;
}



void
Player::state_perform(Player_State _state, State_Function _function)
{
    auto input = pointers->input;
    switch (_state){
      case Player_State::Idle:{
          switch (_function){
            case State_Function::Enter:{
                state_enter_default(this, &GSPRITE->sPlayer_idle, 0, 0);
                physics = ground_physics;
            }break;
            case State_Function::Step:{
                //anim
                anim_index = (float32)input->up.hold;
                //speed
                player_gravity(this);
                player_move_hori(this, false);
                move_collide_tile(&GDATA->tilemap, &pos, &spd, size);

                if (input->jump)
                    state_switch(Player_State::Jump);
                else if (spd.y != 0)
                    state_switch(Player_State::Fall);
                else if (move_input.x != 0 && spd.x != 0)
                    state_switch(Player_State::Walk);
            }break;
          }
      }break;
            
      case Player_State::Walk:{
          switch (_function){
            case State_Function::Enter:{
                
                state_enter_default(this, &GSPRITE->sPlayer_walk);
                physics = ground_physics;
            }break;
            case State_Function::Step:{
                

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
                    state_switch(Player_State::Jump);
                else if (spd.y != 0)
                    state_switch(Player_State::Fall);
                else if (spd.x == 0)
                    state_switch(Player_State::Idle);
            }break;
          }
      }break;

//Air
      case Player_State::Jump:{
          switch (_function){
            case State_Function::Enter:{
                
                state_enter_default(this, &GSPRITE->sPlayer_air, 0, 0);

                physics = jump_physics;
                spd.y = -jump_spd;
            }break;
            case State_Function::Step:{
                

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
                player_gravity(this);
                player_move_hori(this, true);
                if (!input->jump.hold) spd.y *= 0.9f;
                Collide_Data coll = move_collide_tile(&GDATA->tilemap, &pos, &spd, size);

                // state
                if (spd.y > 0)
                    state_switch(Player_State::Fall);

            }break;
          }
      }break;
      case Player_State::Fall:
      {
          switch (_function){
            case State_Function::Enter:
            {
                Sprite* spr = pointers->input->up.hold ? &GSPRITE->sPlayer_air_reach : &GSPRITE->sPlayer_air;
                float32 index = pointers->input->up.hold ? anim_index : 0;
                state_enter_default(this, spr, index, 0);
        
                physics = fall_physics;
            }
            break;
            case State_Function::Step:
            {
                //anim
                if (input->up.hold){
                    sprite = &GSPRITE->sPlayer_air_reach;
                    anim_index = (move_input.x != 0 && move_input.x != scale.x);
                }else{
                    sprite = &GSPRITE->sPlayer_air;
                }
                anim_speed = abs_f32(spd.y) * 0.8f;

        
                player_move_hori(this, true);
                player_gravity(this);
                Collide_Data coll = move_collide_tile(&pointers->data->tilemap, &pos, &spd, size);

                auto st = (spd.x != 0 ? Player_State::Walk : Player_State::Idle);
                if (coll.ydir == 1)
                    state_switch(st);
            }break;
          }
      }break;
        
    }
}
//STATE TEMPLATE
// case Player_State::Jump:{
//     switch (_function){
//       case State_Function::Enter:{
//       }break;
//       case State_Function::Step:{
//       }break;
//       case State_Function::Leave:{
//       }break;
//     }
// }break;



void
Player::Create(Vec2f _pos, Player_State _state)
{
    // *this = Player;
    Player plr; //REQUIRED: dont 0 init
    plr.sprite = &GSPRITE->sPlayer_idle;
    plr.state = _state;
    state_perform(_state, State_Function::Enter);
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

    state_perform(state, State_Function::Step);

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
