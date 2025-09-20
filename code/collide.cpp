/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */

#include "collide.h"

//NOTE: pos_offset - useful for easily passing in a sprite's origin point


//GENERIC
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


//WALL
bool32 collide_wall(Game_Pointers game_pointers, Vec2 pos, Vec2 size, Vec2 pos_offset)
{
    Walls* walls = &game_pointers.entity->walls;
    for (int i = 0; i < walls->count_alive; ++i)
    {
        Vec2 wall_pos = walls->pos[i];
        Vec2 wall_size = walls->size[i];
        if (collide(pos, size, wall_pos, wall_size, pos_offset))
            return true;
    }
    return false;
}

Rectangle collide_wall_rect(Game_Pointers game_pointers, Vec2 pos, Vec2 size, Vec2 pos_offset)
{
    Walls* walls = &game_pointers.entity->walls;
    for (int i = 0; i < walls->count_alive; ++i)
    {
        Vec2 wall_pos = walls->pos[i];
        Vec2 wall_size = walls->size[i];
        if (collide(pos, size, wall_pos, wall_size, pos_offset))
            return {walls->pos[i], walls->size[i]};
    }
    return {};
}


Collide_Data move_collide_wall(Game_Pointers game_pointers, Vec2* pos, Vec2* spd, Vec2 size)
{
	Collide_Data wallCollData = {};

	float entBboxTop = 0;
	float entBboxBottom = 0;
	float entBboxLeft = 0;
	float entBboxRight = 0;
	float wallBboxTop = 0;
	float wallBboxBottom = 0;
	float wallBboxLeft = 0;
	float wallBboxRight = 0;

	//VERTICAL
	pos->y += spd->y;
	entBboxTop = pos->y;
	entBboxBottom = entBboxTop + size.y;
	Rectangle wall = collide_wall_rect(game_pointers, *pos, size, {0.0f, (float32)sign(spd->y)});
    bool32 wall_exists = !(wall.size.x == 0 && wall.size.y == 0);
	if (wall_exists)
	{
		float32 resY = pos->y;
		wallBboxTop = wall.pos.y;
		wallBboxBottom = wallBboxTop + wall.size.y;
		
		if (spd->y > 0){
			resY = MIN(resY, wallBboxTop + pos->y - entBboxBottom);
		}
		else if (spd->y < 0){
			resY = MAX(resY, wallBboxBottom + pos->y - entBboxTop);
		}
		
		wallCollData.hori = true;
		wallCollData.dir.y = (float32)sign(spd->y);
		pos->y = resY;
		spd->y = 0;
	}

	//HORIZONTAL
	pos->x += spd->x;
	entBboxLeft = pos->x;
	entBboxRight = entBboxLeft + size.x;
	wall = collide_wall_rect(game_pointers, *pos, size, {(float32)sign(spd->x), 0.0f});
    wall_exists = !(wall.size.x == 0 && wall.size.y == 0);
	if (wall_exists)
    {
		float32 resX = pos->x;
		wallBboxLeft = wall.pos.x;
		wallBboxRight = wallBboxLeft + wall.size.x;
		if (spd->x > 0){
			resX = MIN(resX, wallBboxLeft + pos->x - entBboxRight);
		}
		else if (spd->x < 0){
			resX = MAX(resX, wallBboxRight + pos->x - entBboxLeft);
		}
		
		wallCollData.hori = true;
		wallCollData.dir.x = (float32)sign(spd->x);
		pos->x = resX;
		spd->x = 0;
	}

	return wallCollData;
}



//MISC
bool32 on_screen(Vec2 pos, Vec2 padding)
{
    return (pos.x + padding.x >= BASE_W ||
            pos.x - padding.x <= 0 ||
            pos.y + padding.y >= BASE_H ||
            pos.y - padding.y <= 0
    );
}
