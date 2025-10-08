#pragma once
#include "my_color.h"
// #include "game.h"


/////STUFF//////
#pragma pack(push, 1)
struct BMP_File_Header
{
    uint16 type;
    uint32 size;
    uint16 reserved1;
    uint16 reserved2;
    uint32 offset;
    uint32 info_size;
    int32  width;
    int32  height;
    uint16 planes;
    uint16 bits_per_pixel;
};
#pragma pack(pop)

struct BMP_Data
{
    int32 bits_per_pixel;
    int32 size_bytes;
    int32 width;
    int32 height;
    uint32* pixels;
};



void draw_pixel(Vec2 pos, Color color);
void draw_rect(Vec2 pos, Vec2 size, Color color);
void draw_line(Vec2 pos_start, Vec2 pos_end);

void draw_line_hori(Vec2 pos_start, Vec2 pos_end);
void draw_line_vert(Vec2 pos_start, Vec2 pos_end);
void draw_line(Vec2 pos_start, Vec2 pos_end);
void draw_line_old(Vec2 pos_start, Vec2 pos_end);
void draw_bmp(BMP_Data* bmp, Vec2 pos, Vec2 scale);
void draw_bmp_part(BMP_Data* bmp, Vec2 pos, Vec2 scale_overall, int32 bmp_drawx, int32 bmp_drawy, int32 bmp_draw_width, int32 bmp_draw_height);

void draw_bmp_1sttry(BMP_Data* bmp, Vec2 pos);
void draw_bmp_pixels(BMP_Data* bmp, Vec2 pos);
