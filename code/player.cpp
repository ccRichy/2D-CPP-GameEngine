/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */

#include "player.h"



Player* Player::Create(Vec2 _pos)
{
    pos   = _pos;
    size  = {Tile(1), Tile(2)};
    color = GREEN;
    state = Player_State::ground;
    return this;
}

void Player::Update(Game_Pointers game_pointers, Game_Input_Map input)
{
    float32 maxspd = 1.0f;
    float32 grav = 0.1f;
    float32 jumpspd = 3.0f;
    
    Vec2 move_input = {(float32)(input.right.hold - input.left.hold),
                       (float32)(input.down.hold - input.up.hold)};
    
    spd.x = move_input.x * maxspd;
    if (move_input.x != 0)
        aim_dir = (int8)move_input.x;
    
    size.y = Tile(2); //reset size
    if (input.shoot.press)
    {
        Bullets* bullets = &game_pointers.entity->bullets;
        Entity_Identity bullet = bullets->Create({pos.x + (size.x/2), pos.y+4},
                                                 {3.0f * aim_dir, 0});
        if (bullet)
            size.y += 2;
    }
    
    switch (state)
    {        
        case Player_State::ground:{
            spd.x *= 2;

            move_collide_wall(game_pointers, &pos, &spd, size);
                
            if (input.jump.press)
            {
                //if (spd.y != 0) spd.y *= (spd.y * 0.1f);
                spd.y -= jumpspd;
                state = Player_State::air;
            }
            else if (!collide_wall(game_pointers, pos, size, {0.f, pos.y+1}))
                state = Player_State::air;
        }break;
        
        case Player_State::air:{
            if (input.jump.release){
                if (spd.y < 0){
                    spd.y /= 2;
                }
                grav *= 2;
            }
            spd.y += grav;

            Collide_Data coll = move_collide_wall(game_pointers, &pos, &spd, size);

            if (coll.ydir == 1)
                state = Player_State::ground;
        }break;
    }
}

void Player::Draw(Game_Pointers game_pointers)
{
    //Draw me :)
    draw_rect(game_pointers, pos, size, color);
    draw_pixel(game_pointers, pos, MAGENTA);
    //DRAW SMILEY FACE
    // float32 smile_size = 4;
    // draw_rect(game_pointers, {pos.x, pos.y}, {smile_size, smile_size * 5}, blue);
    // draw_rect(game_pointers, {pos.x - smile_size, pos.y-smile_size}, {smile_size, smile_size}, red);
    // draw_rect(game_pointers, {pos.x + smile_size*5, pos.y-smile_size}, {smile_size, smile_size}, red);
    // draw_rect(game_pointers, {pos.x, pos.y - (smile_size*3)}, {smile_size, smile_size}, red);
    // draw_rect(game_pointers, {pos.x + (smile_size*4), (pos.y-smile_size*3)}, {smile_size, smile_size}, red);

}
