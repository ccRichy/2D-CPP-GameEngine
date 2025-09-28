#include "render.h"


void draw_pixel(Game_Pointers game_pointers, Vec2 pos, Color color)
{
    Game_Render_Buffer* render = game_pointers.render;
    int8 scale = game_pointers.settings->window_scale;

    //TODO: properly handle pixel truncation/rounding
    int32 x_start = (int32)(pos.x * scale); //NOTE: (int32)pos.x * scale  | pixel rendering (floor)
    int32 y_start = (int32)(pos.y * scale); //      (int32)(pos.x * scale)| sub-pixel rendering
    int32 x_end = x_start + scale;
    int32 y_end = y_start + scale;

    if (x_start < 0) x_start = 0;
    if (y_start < 0) y_start = 0;
    if (x_end > render->width)  x_end = render->width;
    if (y_end > render->height) y_end = render->height;
    
    uint8* row = (uint8*)render->memory + (y_start * render->pitch);
    for (int32 Y = y_start; Y < y_end; ++Y)
    {
        uint32* pixel = (uint32*)row + x_start;
        for (int X = x_start; X < x_end; ++X)
        {
            if (color.a == 255)
            {
                *pixel++ = color_struct_to_uint32(color);
            }
            // else if (color.a == 0) //TODO: OPTIMIZATION: test performance
            // {
            //     continue;
            // }
            else
            {
                Color* __color_prev = (Color*)pixel;
                Color color_prev = {__color_prev->b, __color_prev->g, __color_prev->r, 0};
                
                *pixel++ = color_struct_to_uint32( color_get_transparent(color_prev, color) );
            }
        }
        row += render->pitch;
    }
}


void draw_rect(Game_Pointers game_pointers, Vec2 pos, Vec2 size, Color color)
{
    Game_Render_Buffer* render = game_pointers.render;
    int8 scale = game_pointers.settings->window_scale;

    int32 x_start = (int32)(pos.x * scale);
    int32 y_start = (int32)(pos.y * scale);
    int32 x_end = x_start + (int32)(size.x * scale);
    int32 y_end = y_start + (int32)(size.y * scale);

    if (x_start < 0) x_start = 0;
    if (y_start < 0) y_start = 0;
    if (x_end > render->width)  x_end = render->width;
    if (y_end > render->height) y_end = render->height;

    uint8* row = (uint8*)render->memory + (y_start * render->pitch);
    for (int Y = y_start; Y < y_end; ++Y)
    {
        uint32* pixel = ((uint32*)row + x_start);
        for (int X = x_start; X < x_end; ++X)
        {
            if (color.a == 255)
            {
                *pixel++ = color_struct_to_uint32(color);
            }
            else
            {
                Color* __color_prev = (Color*)pixel;
                Color color_prev = {__color_prev->b, __color_prev->g, __color_prev->r, 0};
                
                *pixel++ = color_struct_to_uint32( color_get_transparent(color_prev, color) );
            }
        }
        row += render->pitch;
    }
}


void draw_line_hori(Game_Pointers game_pointers, Vec2 pos_start, Vec2 pos_end)
{
    auto pos_s = pos_start;
    auto pos_e = pos_end;
    if (pos_start.x > pos_end.x)
    {
        pos_s = pos_end;
        pos_e = pos_start;
    }
        
    auto dx = pos_e.x - pos_s.x;
    auto dy = pos_e.y - pos_s.y;

    auto dir = dy < 0 ? -1 : 1;
    dy *= dir;
    
    if (dx != 0)
    {
        auto _y = pos_s.y;
        auto p = 2*dy - dx;
        for (int i = 0; i < dx+1; ++i)
        {
            Vec2 draw_pos = { pos_s.x + i, _y };
            draw_pixel(game_pointers, draw_pos, WHITE);
            if (p >= 0)
            {
                _y += dir;
                p -= 2*dx;
            }
            p += 2*dy;
        }
    }
}
void draw_line_vert(Game_Pointers game_pointers, Vec2 pos_start, Vec2 pos_end)
{
    auto pos_s = pos_start;
    auto pos_e = pos_end;
    if (pos_start.y > pos_end.y)
    {
        pos_s = pos_end;
        pos_e = pos_start;
    }
        
    auto dx = pos_e.x - pos_s.x;
    auto dy = pos_e.y - pos_s.y;

    auto dir = dx < 0 ? -1 : 1;
    dx *= dir;
    
    if (dy != 0)
    {
        auto _x = pos_s.x;
        auto p = 2*dx - dy;
        for (int i = 0; i < dy+1; ++i)
        {
            Vec2 draw_pos = { _x, pos_s.y + i };
            draw_pixel(game_pointers, draw_pos, WHITE);
            if (p >= 0)
            {
                _x += dir;
                p -= 2*dy;
            }
            p += 2*dx;
        }
    }
}
void draw_line(Game_Pointers game_pointers, Vec2 pos_start, Vec2 pos_end)
{
    if ( abs_f(pos_end.x - pos_start.x) > abs_f(pos_end.y - pos_start.y) )
        draw_line_hori(game_pointers, pos_start, pos_end);
    else
        draw_line_vert(game_pointers, pos_start, pos_end);
}

//inefficient but simpler? NOTE: STRESS TEST
void draw_line_old(Game_Pointers game_pointers, Vec2 pos_start, Vec2 pos_end)
{
    int8 scale = game_pointers.settings->window_scale;

    float32 deltaX = pos_end.x - pos_start.x;
    float32 deltaY = pos_end.y - pos_start.y;
    float32 max_delta = MAX( abs_f(deltaX), abs_f(deltaY) );
    if (max_delta != 0)
    {
        //float32 slope = deltaY / deltaX;
        auto stepX = deltaX/max_delta;
        auto stepY = deltaY/max_delta;

        for (int i = 0; i < (int32)max_delta+1; ++i)
        {
            Vec2 draw_pos = {round_f32(pos_start.x + (i * stepX)), round_f32(pos_start.y + (i * stepY))};
            draw_rect(game_pointers, draw_pos, {1, 1}, WHITE);
        }
    }
}
