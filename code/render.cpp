// #include "game.h"
#include "render.h"

/*
  TODO:
  - find some way to facilitate skipping the function if we know it would attempt to draw outside the buffer
  - !!optimize!!
 */

//helper
inline float32 //NOTE: used to scale things in the world agnostic with the zoom
game_get_static_size(Game_Pointers* game_pointers, float32 value)
{
    float32 result = value / game_pointers->settings->zoom_scale;
    return result;
}
inline float32
game_get_draw_scale(Game_Pointers* game_pointers)
{
    auto draw_mode = game_pointers->data->draw_mode;
    float32 result = -1.0f;
    
    if (draw_mode == Draw_Mode::world)
        result = game_pointers->settings->window_scale * game_pointers->settings->zoom_scale;
    else if (draw_mode == Draw_Mode::gui)
        result = game_pointers->settings->window_scale;
    
    return result;
}
inline Vec2
game_get_draw_pos(Game_Pointers* game_pointers)
{
    auto draw_mode = game_pointers->data->draw_mode;
    Vec2 result = {};

    if (draw_mode == Draw_Mode::world)
        result = game_pointers->data->camera_pos * Vec2{-1, -1};
    else if (draw_mode == Draw_Mode::gui)
        result = {};
    
    return result;
    
}



//PRIMITIVES
void
draw_pixel(Game_Pointers* game_pointers, Vec2 pos, Color color)
{
    Game_Render_Buffer* render = game_pointers->render;
    
    float32 scale = game_get_draw_scale(game_pointers);
    pos          += game_get_draw_pos(game_pointers);
        
    //TODO: properly handle pixel truncation/rounding
    int32 x_start = round_i32(pos.x * scale); //NOTE: (int32)pos.x * scale  | pixel rendering (floor)
    int32 y_start = round_i32(pos.y * scale); //      (int32)(pos.x * scale)| sub-pixel rendering
    int32 x_end =   round_i32(x_start + scale);
    int32 y_end =   round_i32(y_start + scale);

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

void
draw_rect(Game_Pointers* game_pointers, Vec2 pos, Vec2 size, Color color)
{
    Game_Render_Buffer* render = game_pointers->render;
    
    float32 scale = game_get_draw_scale(game_pointers);
    pos          += game_get_draw_pos(game_pointers);

    int32 x_start = round_i32(pos.x * scale);
    int32 x_end =   round_i32(x_start + (size.x * scale));
    int32 y_start = round_i32(pos.y * scale);
    int32 y_end =   round_i32(y_start + (size.y * scale));

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

void
draw_line_hori(Game_Pointers* game_pointers, Vec2 pos_start, Vec2 pos_end, Color color)
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
            draw_pixel(game_pointers, draw_pos, color);
            if (p >= 0)
            {
                _y += dir;
                p -= 2*dx;
            }
            p += 2*dy;
        }
    }
}
void
draw_line_vert(Game_Pointers* game_pointers, Vec2 pos_start, Vec2 pos_end, Color color)
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
            draw_pixel(game_pointers, draw_pos, color);
            if (p >= 0)
            {
                _x += dir;
                p -= 2*dy;
            }
            p += 2*dx;
        }
    }
}
void
draw_line(Game_Pointers* game_pointers, Vec2 pos_start, Vec2 pos_end, Color color)
{
    if ( abs_f32(pos_end.x - pos_start.x) > abs_f32(pos_end.y - pos_start.y) )
        draw_line_hori(game_pointers, pos_start, pos_end, color);
    else
        draw_line_vert(game_pointers, pos_start, pos_end, color);
}
//less efficent but simpler? NOTE: benchmark performance
void
draw_line_old(Game_Pointers* game_pointers, Vec2 pos_start, Vec2 pos_end)
{
    int32 scale = (int32)game_pointers->settings->window_scale;

    float32 deltaX = pos_end.x - pos_start.x;
    float32 deltaY = pos_end.y - pos_start.y;
    float32 max_delta = MAX( abs_f32(deltaX), abs_f32(deltaY) );
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





//IMAGES
void
draw_bmp(Game_Pointers* game_pointers, BMP_File* bmp, Vec2 pos, Vec2 scale)
{
    //NOTE: this code was partially generated by ai
    Game_Render_Buffer* render = game_pointers->render;
    
    float32 draw_scale = game_get_draw_scale(game_pointers);
    float32 xscale_final = abs_f32(draw_scale * scale.x);
    float32 yscale_final = abs_f32(draw_scale * scale.y);
    pos +=          game_get_draw_pos(game_pointers);
    pos.y += bmp->height * abs_f32(scale.y);
    
    int32 x_start = round_i32(pos.x * draw_scale);
    int32 x_end   = round_i32(x_start + (bmp->width * xscale_final));
    int32 y_start = round_i32(pos.y * draw_scale);
    int32 y_end   = round_i32(y_start - (bmp->height * yscale_final));
    int32 y_offscreen_amt = 0;
    
    if (x_start < 0)              x_start = 0;
    if (x_end > render->width)    x_end = render->width;
    if (y_end < 0)                y_end = 0;
    if (y_start > render->height){
        y_offscreen_amt = y_start - render->height;
        y_start = render->height;
    }
        

    uint32* bmp_pixels = (uint32*)((uint8*)bmp + bmp->offset);

    for (int Y = y_end; Y < y_start; ++Y)
    {
        if (Y >= render->height) continue; //preventing crash at bottom of buffer

        uint32* pixel = (uint32*)((uint8*)render->memory + Y * render->pitch);
        for (int X = x_start; X < x_end; ++X)
        {
            // Map screen pixel -> source bitmap pixel
            int32 bmp_x;
            int32 bmp_y;
            
            if (sign(scale.x) == 1)
                 bmp_x = (int32)(((X - pos.x * draw_scale)) / xscale_final);
            else bmp_x = bmp->width-1 - (int32)(((X - pos.x * draw_scale)) / xscale_final);
                
            if (sign(scale.y) == 1)
                 bmp_y = (int32)((y_start - 1 - Y + y_offscreen_amt) / yscale_final);
            else bmp_y = bmp->height - (int32)((y_start - 1 - Y + y_offscreen_amt) / yscale_final);

            uint32 color_new = bmp_pixels[bmp_y * bmp->width + bmp_x];
            float32 alpha = (float32)((color_new >> 24) & 0xFF) / 255.f;

            if (alpha > 0.0f)
            {
                uint32 color_prev = pixel[X];
                uint32 target_color = (
                    color_channel_get_transparent((color_prev >> 16) & 0xFF, (color_new >> 16) & 0xFF, alpha) << 16 |
                    color_channel_get_transparent((color_prev >> 8) & 0xFF,  (color_new >> 8) & 0xFF,  alpha) << 8 |
                    color_channel_get_transparent((color_prev & 0xFF),       (color_new & 0xFF),       alpha)
                );

                pixel[X] = target_color;
            }
        }
    }
}


//TODO: change origin point to top-left? currently its bottom-left (thanks bmp)
void
draw_bmp_part(Game_Pointers* game_pointers, BMP_File* bmp, Vec2 pos, Vec2 scale_overall, int32 bmp_drawx, int32 bmp_drawy, int32 bmp_draw_width, int32 bmp_draw_height)
{
    //NOTE: this code was partially generated by ai
    Game_Render_Buffer* render = game_pointers->render;
    float32 draw_scale = game_get_draw_scale(game_pointers);
    float32 xscale_final = abs_f32(draw_scale * scale_overall.x);
    float32 yscale_final = abs_f32(draw_scale * scale_overall.y);
    pos                 += game_get_draw_pos(game_pointers);


    pos.y += bmp->height * scale_overall.y;
    if (sign(scale_overall.x) == -1)
        pos.x -= bmp->width - bmp_draw_width;

    int32 x_left = round_i32(pos.x * draw_scale);
    int32 x_right   = round_i32(x_left + (bmp->width * xscale_final));
    int32 y_top = round_i32(pos.y * draw_scale);
    int32 y_bottom   = round_i32(y_top - (bmp->height * yscale_final));

    int32 y_offscreen_amt = 0;
    if (x_left < 0)              x_left = 0;
    if (x_right > render->width) x_right = render->width;
    //TODO: THIS IS WRONG vv fix and test later
    if (y_bottom < 0)            y_bottom = 0;
    if (y_top > render->height){
        y_offscreen_amt = y_top - render->height;
        y_top = render->height;
    }
        

    uint32* bmp_pixels = (uint32*)((uint8*)bmp + bmp->offset);
    int32 bmp_x = bmp_drawx; //pos on bmp from which to draw drawing
    int32 bmp_y = bmp_drawy;
    for (int32 Y = y_bottom; Y < y_top; ++Y)
    {
        if (Y >= render->height) continue;

        uint32* buffer_pixel = (uint32*)((uint8*)render->memory + (Y * render->pitch));
        bmp_x = bmp_drawx;
        
        for (int32 X = x_left; X < x_right; ++X)
        {            
            if (sign(scale_overall.x) == 1)
                 bmp_x = bmp_drawx + (int32)((X - (pos.x * draw_scale)) / xscale_final);
            else bmp_x = bmp->width-1 + bmp_drawx - (int32)(((X) - (pos.x * draw_scale)) / xscale_final);
                
            if (sign(scale_overall.y) == 1)
                 bmp_y = bmp_drawy + (int32)((y_top - 1 - Y + y_offscreen_amt) / yscale_final);
            else bmp_y = bmp_drawy + bmp->height - (int32)((y_top - 1 - Y + y_offscreen_amt) / yscale_final);

            if (bmp_x >= bmp_drawx + bmp_draw_width)
                continue;
            if (bmp_y < bmp->height - bmp_draw_height)
                continue;
            
            uint32 color_new = bmp_pixels[bmp_y * bmp->width + bmp_x];
            float32 alpha = (float32)((color_new >> 24) & 0xFF) / 255.f;

            if (alpha > 0.0f)
            {
                uint32 color_prev = buffer_pixel[X];
                uint32 target_color = (
                    color_channel_get_transparent((color_prev >> 16) & 0xFF, (color_new >> 16) & 0xFF, alpha) << 16 |
                    color_channel_get_transparent((color_prev >> 8) & 0xFF,  (color_new >> 8) & 0xFF,  alpha) << 8 |
                    color_channel_get_transparent((color_prev & 0xFF),       (color_new & 0xFF),       alpha)
                );

                buffer_pixel[X] = target_color;
            }
        }
        bmp_y--;
    }
}





void
draw_bmp_1sttry(Game_Pointers* game_pointers, BMP_File* bmp, Vec2 pos)
{
    Game_Render_Buffer* render = game_pointers->render;
    int32 scale = (int32)game_pointers->settings->window_scale;
    
    pos.y += bmp->height;
    int32 x_start = round_i32(pos.x * scale);
    int32 x_end   = x_start + (bmp->width * scale);
    int32 x_offscreen_amt = 0;
    int32 y_start = round_i32(pos.y * scale) + (scale-1);
    int32 y_end   = y_start - (bmp->height * scale);
    
    if (x_start < 0){
        x_offscreen_amt = -x_start; //NOTE: flipping sign
        x_start = 0;
    }
    if (x_end > render->width) x_end      = render->width;
    if (y_end < 0) y_end                  = 0;
    if (y_start > render->height) y_start = render->height;

    float32 offscreen_src_amt = (float32)x_offscreen_amt / (float32)scale;
    int32 bitmap_pixel_offset = (int32)offscreen_src_amt;
    float32 subpixel_offset = offscreen_src_amt - bitmap_pixel_offset;
    
    int32 pixel_offscreen_amt = x_offscreen_amt % scale;
    int32 pixel_increment = 0;
    int32 row_increment = 0;

    uint8* row = (uint8*)render->memory + (y_start * render->pitch);
    uint32* bitmap_pixel = (uint32*)((uint8*)bmp + bmp->offset);
    bitmap_pixel += bitmap_pixel_offset;

    for (int Y = y_end; Y < y_start; ++Y)
    {
        uint32* pixel = ((uint32*)row + x_start);
        for (int X = x_start; X < x_end; ++X)
        {
            float32 alpha = (float32)(*bitmap_pixel >> 24 & 0xFF) / 255.f;
            float32 alphasub = 1 - (alpha);

            uint32 color_prev = *pixel;
            uint32 color_new = *bitmap_pixel;

            uint32 target_color = (
                color_channel_get_transparent((color_prev >> 16 & 0xFF), (color_new >> 16 & 0xFF), alpha) << 16 |
                color_channel_get_transparent((color_prev >> 8 & 0xFF), (color_new >> 8 & 0xFF), alpha) << 8 |
                color_channel_get_transparent((color_prev & 0xFF), (color_new & 0xFF), alpha)
            );

            *pixel++ = target_color;

            if (X == x_start)
            {
                pixel_increment = pixel_offscreen_amt + 1;
            }
            else if (++pixel_increment >= scale)
            {
                pixel_increment = 0;
                bitmap_pixel++;
            }
        }

        if (++row_increment >= scale)
        {
            bitmap_pixel += bmp->width;
            row_increment = 0;
        }
        bitmap_pixel -= (bmp->width - (int32)((float32)x_offscreen_amt / scale));
        row -= render->pitch;
    }
}

void
draw_bmp_pixels(Game_Pointers* game_pointers, BMP_File* bmp, Vec2 pos)
{
    pos.y += bmp->height;
    uint32* bitmap_pixel = (uint32*)((uint8*)bmp + bmp->offset);
    
    for (int pixel_index = 0; pixel_index < bmp->width * bmp->height; ++pixel_index)
    {
        Color color_cast = *(Color*)bitmap_pixel;
        Color color_converted = {color_cast.b, color_cast.g, color_cast.r, color_cast.a};

        int32 xoffset = pixel_index % bmp->width;
        float32 draw_x = pos.x + xoffset;
        draw_pixel(game_pointers, {draw_x, pos.y}, color_converted);

        if (xoffset >= bmp->width-1)
            pos.y -= 1;
        bitmap_pixel++;
    }
}
