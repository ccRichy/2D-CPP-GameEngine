#pragma once
#include "my_color.h"
#include "game.h"




void draw_pixel(Game_Pointers game_pointers, Vec2 pos, Color color);
void draw_rect(Game_Pointers game_pointers, Vec2 pos, Vec2 size, Color color);
void draw_line(Game_Pointers game_pointers, Vec2 pos_start, Vec2 pos_end);

void draw_line_hori(Game_Pointers game_pointers, Vec2 pos_start, Vec2 pos_end);
void draw_line_vert(Game_Pointers game_pointers, Vec2 pos_start, Vec2 pos_end);
void draw_line(Game_Pointers game_pointers, Vec2 pos_start, Vec2 pos_end);
void draw_line_old(Game_Pointers game_pointers, Vec2 pos_start, Vec2 pos_end);
void draw_bmp(Game_Pointers game_pointers, BMP_File* bmp, Vec2 pos, Vec2 scale);
void draw_bmp_part(Game_Pointers game_pointers, BMP_File* bmp, Vec2 pos, Vec2 scale, int32 bmp_drawx, int32 bmp_drawy, int32 bmp_draw_width, int32 bmp_draw_height);

void draw_bmp_1sttry(Game_Pointers game_pointers, BMP_File* bmp, Vec2 pos);
void draw_bmp_pixels(Game_Pointers game_pointers, BMP_File* bmp, Vec2 pos);
