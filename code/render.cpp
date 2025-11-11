// #include "render.h"


//TODO: optimize!!
BMP_Data
DEBUG_load_bmp(const char* filename)
{
    BMP_Data result = {};

    char file_path[64] = {};
    string_append(file_path, "images\\");
    string_append(file_path, filename);
    string_append(file_path, ".bmp");
    auto file = (BMP_File_Header*)pointers->memory->DEBUG_platform_file_read_entire(file_path).memory;
    
    result.size_bytes = file->size;
    result.width = file->width;
    result.height = file->height;
    result.pixels = (u32*)((u8*)file + file->offset);
    result.bits_per_pixel = file->bits_per_pixel;
        
    Assert(result.size_bytes > 0);
    // GMEMORY->DEBUG_platform_file_free_memory(file);
    return result;
};


Sprite
sprite_create(const char* bmp_filename, uint32 frame_num, float32 fps, Vec2f origin)
{
    Sprite result = {};
    result.bmp = DEBUG_load_bmp(bmp_filename);
    result.origin = origin;
    result.fps = fps;
    result.frame_num = frame_num;
    result.is_animation = (frame_num > 0 && fps != 0);
    result.width = result.bmp.width;
    result.height = result.bmp.height;
    result.frame_width = (int32)(result.bmp.width / result.frame_num);
    result.frame_height = result.bmp.height;
    return result;
}

//helper
inline float32 //NOTE: use to keep a scale value the same regardless of zoom
scale_get_zoom_agnostic(float32 scale_value)
{
    float32 result = scale_value / pointers->settings->zoom_scale;
    return result;
}

float32
scale_get_screen_agnostic(float32 scale_value)
{
    float32 result = scale_value / pointers->settings->window_scale;
    return result;    
}

inline float32
game_get_draw_scale(Draw_Mode override_mode = Draw_Mode::Null)
{
    float32 result = -1.0f;
    auto draw_mode = pointers->data->draw_mode;
    if (override_mode != Draw_Mode::Null)
        draw_mode = override_mode;
    
    if (draw_mode == Draw_Mode::World)
        result = pointers->settings->window_scale * pointers->settings->zoom_scale;
    else if (draw_mode == Draw_Mode::Gui)
        result = pointers->settings->window_scale;
    
    return result;
}
inline Vec2f
game_get_draw_pos(Draw_Mode override_mode = Draw_Mode::Null)
{
    Vec2f result = {};
    auto draw_mode = pointers->data->draw_mode;
    if (override_mode != Draw_Mode::Null)
        draw_mode = override_mode;

    if (draw_mode == Draw_Mode::World)
        result = pointers->data->camera_pos * Vec2f{-1, -1};
    else if (draw_mode == Draw_Mode::Gui)
        result = {};
    
    return result;
    
}



//PRIMITIVES
void
draw_pixel(Vec2f pos, Color color)
{
    Game_Render_Buffer* render = pointers->render;

    float32 scale = game_get_draw_scale();
    pos          += game_get_draw_pos();

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
                *pixel++ = color.decimal;//color_struct_to_uint32(color);
            }
            // else if (color.a == 0) //TODO: OPTIMIZATION: test performance
            // {
            //     continue;
            // }
            else
            {
                Color color_new = color_get_transparent(*(Color*)pixel, (Color)color);
                *pixel++ = color_new.decimal;
            }
        }
        row += render->pitch;
    }
}

void
draw_rect(Vec2f pos, Vec2f size, Color color)
{
    Game_Render_Buffer* render = pointers->render;
    
    float32 scale = game_get_draw_scale();
    pos          += game_get_draw_pos();

    int32 x_left =   round_i32(pos.x * scale);
    int32 x_right =  round_i32(x_left + (size.x * scale));
    int32 y_top =    round_i32(pos.y * scale);
    int32 y_bottom = round_i32(y_top + (size.y * scale));

    if (x_left < 0) x_left = 0;
    if (x_right > render->width)  x_right = render->width;
    if (y_top < 0) y_top = 0;
    if (y_bottom > render->height) y_bottom = render->height;

    uint8* row = (uint8*)render->memory + (y_top * render->pitch);
    for (int Y = y_top; Y < y_bottom; ++Y)
    {
        uint32* pixel = ((uint32*)row + x_left);
        for (int X = x_left; X < x_right; ++X)
        {
            if (color.a == 255)
            {
                *pixel++ = color.decimal;//color_struct_to_uint32(color);
            }
            else
            {
                Color color_new = color_get_transparent(*(Color*)pixel, (Color)color);
                *pixel++ = color_new.decimal;
            }
        }
        row += render->pitch;
    }
}
void
draw_rect(Rectangle rect, Color color)
{
    draw_rect(rect.pos, rect.size, color);
}

void draw_rect_outline()
{
    
}



void
draw_line_hori(Vec2f pos_start, Vec2f pos_end, Color color)
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
            Vec2f draw_pos = { pos_s.x + i, _y };
            draw_pixel(draw_pos, color);
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
draw_line_vert(Vec2f pos_start, Vec2f pos_end, Color color)
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
            Vec2f draw_pos = { _x, pos_s.y + i };
            draw_pixel(draw_pos, color);
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
draw_line(Vec2f pos_start, Vec2f pos_end, Color color)
{
    if ( abs_f32(pos_end.x - pos_start.x) > abs_f32(pos_end.y - pos_start.y) )
        draw_line_hori(pos_start, pos_end, color);
    else
        draw_line_vert(pos_start, pos_end, color);
}

void
draw_line_old(Vec2f pos_start, Vec2f pos_end)
{
    //less efficent but simpler? NOTE: benchmark performance
    int32 scale = (int32)pointers->settings->window_scale;

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
            Vec2f draw_pos = {round_f32(pos_start.x + (i * stepX)), round_f32(pos_start.y + (i * stepY))};
            draw_rect(draw_pos, {1, 1}, WHITE);
        }
    }
}





//IMAGES
void
draw_bmp(BMP_Data* bmp, Vec2f pos, Vec2f scale)
{
    //NOTE: this code was partially generated by ai
    Game_Render_Buffer* render = pointers->render;
    
    float32 draw_scale = game_get_draw_scale();
    float32 xscale_final = abs_f32(draw_scale * scale.x);
    float32 yscale_final = abs_f32(draw_scale * scale.y);
    pos +=          game_get_draw_pos();
    pos.y += bmp->height * abs_f32(scale.y);
    
    int32 x_start = round_i32(pos.x * draw_scale);
    int32 x_end   = round_i32(x_start + (bmp->width * xscale_final));
    int32 y_start = round_i32(pos.y * draw_scale);
    int32 y_end   = round_i32(y_start - (bmp->height * yscale_final));
    int32 y_offscreen_amt = 0;
    
    if (x_start < 0)
        x_start = 0;
    if (x_end > render->width)
        x_end = render->width;
    if (y_end < 0)
        y_end = 0;
    if (y_start > render->height){
        y_offscreen_amt = y_start - render->height;
        y_start = render->height;
    }
        

    uint32* bmp_pixels = bmp->pixels;

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



void
draw_bmp_part_old(BMP_Data* bmp, Vec2f pos, Vec2f scale_overall, int32 bmp_drawx, int32 bmp_drawy, int32 bmp_draw_width, int32 bmp_draw_height)
{
    //NOTE: this code was partially generated by ai
    Game_Render_Buffer* render = pointers->render;
    float32 draw_scale = game_get_draw_scale();
    float32 xscale_final = abs_f32(draw_scale * scale_overall.x);
    float32 yscale_final = abs_f32(draw_scale * scale_overall.y);
    pos                 += game_get_draw_pos();


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
        

    uint32* bmp_pixels = bmp->pixels;
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
draw_bmp_part(BMP_Data* bmp, Vec2f pos, Vec2f scale_overall, int32 bmp_drawx, int32 bmp_drawy, int32 bmp_draw_width, int32 bmp_draw_height)
{
    //NOTE: this code was partially generated by ai
    Game_Render_Buffer* render = pointers->render;
    float32 draw_scale = game_get_draw_scale();
    float32 xscale_final = abs_f32(draw_scale * scale_overall.x);
    float32 yscale_final = abs_f32(draw_scale * scale_overall.y);
    pos                 += game_get_draw_pos();


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
        

    uint32* bmp_pixels = bmp->pixels;
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
draw_bmp_1sttry(BMP_Data* bmp, Vec2f pos)
{
    Game_Render_Buffer* render = pointers->render;
    int32 scale = (int32)pointers->settings->window_scale;
    
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
    uint32* bitmap_pixel = bmp->pixels;
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




inline void
draw_sprite(Sprite* sprite, Vec2f pos, Vec2f scale)
{
    draw_bmp(&sprite->bmp, pos - sprite->origin, scale);
}
inline void
draw_sprite_part(Sprite* sprite, Vec2f pos, Vec2f scale_overall, Vec2i img_drawpos, Vec2i img_drawsize)
{
    draw_bmp_part(&sprite->bmp, pos - sprite->origin, scale_overall, img_drawpos.x, img_drawpos.y, img_drawsize.x, img_drawsize.y);
}
inline void
draw_sprite_frame(Sprite* spr, Vec2f pos, float32 anim_index, Vec2f scale = {1, 1})
{
    int32 frame = floor_i32(anim_index);
    draw_sprite_part(spr, pos, scale,
                     {frame * spr->frame_width, 0}, //pos
                     {spr->frame_width, spr->frame_height });//size
}





// void
// draw_bmp_pixels(BMP_Data* bmp, Vec2f pos)
// {
//     pos.y += bmp->height;
//     uint32* bitmap_pixel = bmp->pixels;
    
//     for (int pixel_index = 0; pixel_index < bmp->width * bmp->height; ++pixel_index)
//     {
//         Color color_cast = *(Color*)bitmap_pixel;
//         Color color_converted = {color_cast.b, color_cast.g, color_cast.r, color_cast.a};

//         int32 xoffset = pixel_index % bmp->width;
//         float32 draw_x = pos.x + xoffset;
//         draw_pixel({draw_x, pos.y}, color_converted);

//         if (xoffset >= bmp->width-1)
//             pos.y -= 1;
//         bitmap_pixel++;
//     }
// }




//EXTRA
// internal void
// render_weird_gradient(Game_Render_Buffer* buffer, int blue_offset, int green_offset)
// {
//     uint8* row = (uint8*)buffer->memory; //type = separation
    
//     for (int Y = 0; Y < buffer->height; ++Y)
//     {
//         uint8* pixel = (uint8*)row; //reference to individal pixel 0xBBGRRXX //endian
//         for (int X = 0; X < buffer->width; ++X)
//         {
//             *pixel = (uint8)(X%2 + blue_offset);
//             pixel++;

//             *pixel = (uint8)(Y + green_offset);
//             pixel++;

//             *pixel = 0;
//             pixel++;

//             *pixel = 0;
//             pixel++;
//         }

//         row += buffer->pitch;
//     }
// }
