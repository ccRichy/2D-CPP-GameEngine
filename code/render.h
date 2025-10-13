#pragma once
// #include "my_color.h"


/////STUFF//////
#pragma pack(push, 1)
struct BMP_File_Header
{   //NOTE: we might need to get the RGB layout *potentially*
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

struct Sprite
{
    BMP_Data bmp;
	Vec2f origin;
    int32 width;
    int32 height;
    float32 fps;
    uint32 frame_num;
    bool32 is_animation;
};


Sprite sprite_create(const char* bmp_filename, uint32 frame_num = 1, float32 fps = 1, Vec2f origin = {});

void draw_pixel(Vec2f pos, Color color);
void draw_rect(Vec2f pos, Vec2f size, Color color);
void draw_line(Vec2f pos_start, Vec2f pos_end);

void draw_line_hori(Vec2f pos_start, Vec2f pos_end);
void draw_line_vert(Vec2f pos_start, Vec2f pos_end);
void draw_line(Vec2f pos_start, Vec2f pos_end);
void draw_line_old(Vec2f pos_start, Vec2f pos_end);
void draw_bmp(BMP_Data* bmp, Vec2f pos, Vec2f scale = {1, 1});
void draw_bmp_part(BMP_Data* bmp, Vec2f pos, Vec2f scale_overall, int32 bmp_drawx, int32 bmp_drawy, int32 bmp_draw_width, int32 bmp_draw_height);

void draw_bmp_1sttry(BMP_Data* bmp, Vec2f pos);
void draw_bmp_pixels(BMP_Data* bmp, Vec2f pos);

inline void draw_sprite(Sprite* sprite, Vec2f pos, Vec2f scale = {1, 1});
inline void draw_sprite_part(Sprite* sprite, Vec2f pos, Vec2f scale_overall, Vec2i img_drawpos, Vec2i img_drawsize);
