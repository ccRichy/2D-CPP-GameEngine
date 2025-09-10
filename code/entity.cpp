/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */
#include "entity.h"
#include "render.h"


Walls* Walls::Create(Vec2 _pos, Vec2 _size, My_Color _color)
{
    pos[count] = _pos;
    size[count] = _size;
    color[count] = _color;
    count++;

    return this;
}

void Walls::Draw(Game_Data_Pointers* game_data)
{
    for (int i = 0; i < count; ++i)
    {
        draw_rect(game_data, pos[i], {size[i].x, size[i].y}, color[i]);
    }
}
