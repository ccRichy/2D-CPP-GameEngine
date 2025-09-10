/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */
#include "player.h"

//NOTE: pos_offset - useful for easily passing in a sprite's origin point

bool32 collide(Vec2 pos1, Vec2 size1, Vec2 pos2, Vec2 size2, Vec2 pos_offset)
{
    float x1 = pos1.x + pos_offset.x;
	float y1 = pos1.y + pos_offset.y;
	float bboxLeft1 = x1;
	float bboxRight1 = x1 + size1.x;
	float bboxTop1 = y1;
	float bboxBottom1 = y1 + size1.y;
	
	float x2 = pos2.x;
	float y2 = pos2.y;
	float bboxLeft2 = x2;
	float bboxRight2 = x2 + size2.x;
	float bboxTop2 = y2;
	float bboxBottom2 = y2 + size2.y;

	bool hori = bboxRight1 > bboxLeft2 && bboxLeft1 < bboxRight2;
	bool vert = bboxBottom1 > bboxTop2 && bboxTop1 < bboxBottom2;

	return (vert && hori);
}

bool32 collide_wall(Game_Data_Pointers* game_data, Vec2 pos, Vec2 size, Vec2 pos_offset)
{
    Walls* walls = &game_data->state->walls;
    for (int i = 0; i < walls->count; ++i)
    {
        Vec2 wall_pos = walls->pos[i];
        Vec2 wall_size = walls->size[i];
        if (collide(pos, size, wall_pos, wall_size))
            return true;
    }
    return false;
}




Player* Player::Create(Vec2 _pos, Vec2 _size, My_Color _color)
{
    pos   = _pos;
    size  = _size;
    color = _color;
    return this;
}

void Player::Update(Game_Input_Map INP)
{
    float32 maxspd = 1.0f;
    Vec2 movex = {
        (float32)(INP.right.hold - INP.left.hold) * maxspd,
        (float32)(INP.down.hold - INP.up.hold) * maxspd
    };
    spd = movex;
    pos += spd;
}

void Player::Draw(Game_Data_Pointers* game_data)
{
    //test rect
    bool32 is_colliding = collide_wall(game_data, pos, size);
    color = is_colliding ? white : red;
    
    //Draw me :)
    draw_rect(game_data, pos, size, color);
    
    //DRAW SMILEY FACE
    // float32 smile_size = 4;
    // draw_rect(game_data, {pos.x, pos.y}, {smile_size, smile_size * 5}, blue);
    // draw_rect(game_data, {pos.x - smile_size, pos.y-smile_size}, {smile_size, smile_size}, red);
    // draw_rect(game_data, {pos.x + smile_size*5, pos.y-smile_size}, {smile_size, smile_size}, red);
    // draw_rect(game_data, {pos.x, pos.y - (smile_size*3)}, {smile_size, smile_size}, red);
    // draw_rect(game_data, {pos.x + (smile_size*4), (pos.y-smile_size*3)}, {smile_size, smile_size}, red);

}
