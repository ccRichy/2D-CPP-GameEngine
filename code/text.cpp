/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */

//NOTE: Font creation function assumes a vertical image so we can crawl linearly over the pixels

#include "text.h"

Font
font_create(BMP_Data* image, uint32 glyph_size)
{
    Font result = {};

    uint32 glyph_area = glyph_size * glyph_size;
    uint32 image_area = image->width * image->height;
    
    int32 glyph_offset = FONT_LENGTH;
    int32 glyph_size_bytes = glyph_area * (image->bits_per_pixel / 8);
    uint32* image_pixels = image->pixels;

    //loop over the entire image & move image pixels into respective glyph pixels
    uint32 glyph_ticker = 0;
    for (uint32 image_pixel_index = 0; image_pixel_index < image_area; ++image_pixel_index)
    {
        uint32 image_pixel = image_pixels[image_pixel_index];
        Color* DEBUG_color = (Color*)(&image_pixel);
        Glyph* glyph = &pointers->data->font_test.glyphs[glyph_offset];
        
        int32 glyph_x = image_pixel_index % glyph_size;
        //reverse vertical pixel arrangement
        int32 glyph_y = glyph_size-1 - floor_i32((float32)(image_pixel_index % glyph_area) / image->width);
        
        glyph->pixels[glyph_y][glyph_x] = image_pixel;

        if (glyph_offset <= 0)
            return result;
        if (++glyph_ticker >= glyph_area){
            glyph_offset--;
            glyph_ticker = 0;
        }
    }
    
    return result;
}
void
draw_glyph(char text_character, Vec2f pos, Vec2f scale)
{
    auto render = pointers->render;
    int32 glyph_size = 8;

    float32 draw_scale = game_get_draw_scale();
    float32 xscale_final = abs_f32(draw_scale * scale.x);
    float32 yscale_final = abs_f32(draw_scale * scale.y);
    pos               += game_get_draw_pos();


    int32 x_left   = round_i32(pos.x * draw_scale);
    int32 x_right  = round_i32(x_left + (glyph_size * xscale_final));
    int32 y_top    = round_i32(pos.y * draw_scale);
    int32 y_bottom = round_i32(y_top + (glyph_size * yscale_final));

    if (x_left < 0)                 x_left = 0;
    if (x_right > render->width)    x_right = render->width;
    if (y_top < 0)                  y_top = 0;
    if (y_bottom > render->height)  y_bottom = render->height;

    float32 render_pos_yoffset = pos.y;
    uint32* render_pixel = (uint32*)(render->memory);
    render_pixel += (int32)(render_pos_yoffset * render->pitch);
    Glyph* glyph = &pointers->data->font_test.glyphs[text_character - FONT_ASCII_CHARACTER_START_OFFSET];
    //loop through all real coords
    int32 image_pixel_x = 0;
    int32 image_pixel_y = 0;
    for (int32 Y_Index = y_top; Y_Index < y_bottom; ++Y_Index)
    {
        render_pixel = (uint32*)((uint8*)render->memory + (Y_Index * render->pitch));
        
        for (int X_Index = x_left; X_Index < x_right; ++X_Index)
        {
            image_pixel_x = (int32)((X_Index - x_left) / xscale_final);
            image_pixel_y = (int32)((Y_Index - y_top) / yscale_final);

            uint32 color_prev = render_pixel[X_Index];
            uint32 color = glyph->pixels[image_pixel_y][image_pixel_x];

            //send color to buffer
            if (color > 0)
                render_pixel[X_Index] = color;
        }
    }
}
void
draw_text(const char* text, Vec2f pos, Vec2f scale, Vec2f spacing)
{
    Vec2f final_pos = pos;
    int32 text_length = string_length(text);
    int32 new_lines = 0;
    for (int i = 0; i < text_length; ++i)
    {
        char char_to_draw = text[i];
        if (char_to_draw == '\n')
        {
            new_lines++;
            final_pos.x = pos.x;
            final_pos.y += (spacing.y * scale.y);
        }
        else
        {
            draw_glyph(char_to_draw, final_pos, scale);
            final_pos.x += (spacing.x * scale.x);
        }
    }
}

//#include <stdarg.h>
void draw_text_buffer(Vec2f pos, Vec2f scale, Vec2f spacing, const char* fmt, ...)
{
    char buffer[256];
    
    va_list args;
    va_start(args, fmt);
    vsprintf(buffer, fmt, args); // 'v' = takes a va_list
    va_end(args);
    
    draw_text(buffer, pos, scale, spacing);
}






// void
// draw_text_old(const char* text, Vec2f pos, Vec2f scale = {1, 1}, Vec2 spacing = {5, 8})
// {
//     BMP_File* font_image = game_pointers->data->sFont_ASCII_lilliput;
//     int32 frame_size = font_image->height;
//     float32 drawx_offset = 0; //exists for us to handle the position simply
//     int32 loop_length = string_length(text);
//     for (int i = 0; i < loop_length; ++i)
//     {
//         char frame = text[i];
//         if (frame == '\n')
//         {
//             pos.y += (spacing.y * scale.y);
//             drawx_offset = 0;
//             continue;
//         }

//         char frame_offset = frame - FONT_ASCII_CHARACTER_START_OFFSET;
//         draw_bmp_part(font_image,
//                       {pos.x + drawx_offset, pos.y}, //area_pos
//                       {scale.x, scale.y},            //overall scale
//                       frame_offset * frame_size, 0, //pos on image
//                       frame_size, frame_size);      //size to draw
        
//         drawx_offset += (spacing.x * scale.x);
//     }    
// }
