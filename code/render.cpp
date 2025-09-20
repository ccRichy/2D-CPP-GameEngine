#include "render.h"



void draw_rect(Game_Pointers game_pointers, Vec2 pos, Vec2 size, Color color)
{
    int8 scale = game_pointers.settings->window_scale;
    
    int32 x = (int32)(pos.x * scale); //NOTE: (int32)pos.x * scale =   pixel grid rendering
    int32 y = (int32)(pos.y * scale); //      (int32)(pos.x * scale) = sub-pixel rendering
                                      //one pair of parenthesis determines that
    
    uint8* row = (uint8*)game_pointers.render->memory;
    int32 row_offset = (y < 0 ? 0 : y);
    row += (game_pointers.render->pitch * row_offset);
    
    int32 yoffset = (y < 0 ? -y : 0);
    for (int Y = y + yoffset; Y < y + (size.y * (float32)scale); ++Y)
    {
        if (Y >= game_pointers.render->height) break;
        uint32* pixel = (uint32*)row;
        int32 pixel_offset = (x < 0 ? 0 : x);
        pixel += pixel_offset;
        
        int32 xoffset = (x < 0 ? -x : 0);
        for (int X = x + xoffset; X < x + (size.x * (float32)scale); ++X)
        {
            if (X >= game_pointers.render->width) break;
            *pixel++ = color_struct_to_bits(color);
            //*pixel++ = (uint32)color;
            // uint8 Red = 0;
            // uint8 Green = 0;
            // uint8 Blue = 100;
            // *pixel++ = ( (Red << 16) | (Green << 8) | Blue );
        }
        row += game_pointers.render->pitch;
    }
}
