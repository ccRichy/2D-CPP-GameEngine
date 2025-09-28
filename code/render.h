#pragma once
#include "my_color.h"
#include "game.h"

//NOTE: draw_pixel crashes outside bounds
//      draw_rect draws full-sized pixels

void draw_pixel(Game_Pointers game_pointers, Vec2 pos, Color color);
void draw_rect(Game_Pointers game_pointers, Vec2 pos, Vec2 size, Color color);
void draw_line(Game_Pointers game_pointers, Vec2 pos_start, Vec2 pos_end);

void draw_line_hori(Game_Pointers game_pointers, Vec2 pos_start, Vec2 pos_end);
void draw_line_vert(Game_Pointers game_pointers, Vec2 pos_start, Vec2 pos_end);
void draw_line(Game_Pointers game_pointers, Vec2 pos_start, Vec2 pos_end);
void draw_line_old(Game_Pointers game_pointers, Vec2 pos_start, Vec2 pos_end);
