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
    origin = {size.x/2, size.y};
    return this;
}

void Player::Update(Game_Pointers game_pointers, Game_Input_Map INP)
{
    float32 maxspd = 1.0f;
    float32 grav = 0.1f;
    float32 jumpspd = 3.0f;
    
    Vec2 move_input = {(float32)(INP.right.hold - INP.left.hold),
                       (float32)(INP.down.hold - INP.up.hold)};
    
    spd.x = move_input.x * maxspd;
    if (move_input.x != 0)
        aim_dir = (int8)move_input.x;
    
    size.y = Tile(2); //reset size
    if (INP.shoot.press)
    {
        Bullets* bullets = &game_pointers.entity->bullets;
        Entity_Identity bullet = bullets->Create({pos.x + (size.x/2), pos.y+4},
                                                 {3.0f * aim_dir, 0});
        if (bullet)
            size.y += 2;
    }
    
    switch (state)
    {        
        case Player_States::ground:{
            spd.x *= 2;
            spd.y += grav;
            
            move_collide_wall(game_pointers, &pos, &spd, size);
            
            if (INP.jump.press)
            {
                //if (spd.y != 0) spd.y *= (spd.y * 0.1f);
                spd.y -= jumpspd;
                state = Player_States::air;
            }
        }break;
        
        case Player_States::air:{
            if (INP.jump.release){
                if (spd.y < 0)
                {
                    spd.y /= 2;
                 }
                grav *= 2;
            }
            spd.y += grav;

//            pos += spd;
            Collide_Data coll = move_collide_wall(game_pointers, &pos, &spd, size);

            if (coll.dir.y == 1.0f)
            {
                state = Player_States::ground;
            }
        }break;
    }
    
}

void Player::Draw(Game_Pointers game_pointers)
{
    //test rect
    bool32 is_colliding = collide_wall(game_pointers, pos, size);
    // color = is_colliding ? white : red;
    
    //Draw me :)
    draw_rect(game_pointers, pos, size, color);
    
    //DRAW SMILEY FACE
    // float32 smile_size = 4;
    // draw_rect(game_pointers, {pos.x, pos.y}, {smile_size, smile_size * 5}, blue);
    // draw_rect(game_pointers, {pos.x - smile_size, pos.y-smile_size}, {smile_size, smile_size}, red);
    // draw_rect(game_pointers, {pos.x + smile_size*5, pos.y-smile_size}, {smile_size, smile_size}, red);
    // draw_rect(game_pointers, {pos.x, pos.y - (smile_size*3)}, {smile_size, smile_size}, red);
    // draw_rect(game_pointers, {pos.x + (smile_size*4), (pos.y-smile_size*3)}, {smile_size, smile_size}, red);

}
