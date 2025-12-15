/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */

//TODO: text alignment
#define FONT_GLYPH_SIZE 8
#define FONT_LENGTH 94
#define FONT_ASCII_CHARACTER_START_OFFSET 32



struct Glyph
{
    uint32 pixels[FONT_GLYPH_SIZE][FONT_GLYPH_SIZE];
};
struct Font
{
    //TODO: use my union macro

    Vec2iUnion(glyph_size, glyph_width, glyph_height);
    // Vec2i glyph_size;
    // int32 glyph_width;
    // int32 glyph_height;
    Glyph glyphs[FONT_LENGTH];
};


Font font_create(BMP_Data* image, uint32 glyph_size);

void draw_glyph(char text_character, Vec2f pos, Vec2f scale);
void draw_text(const char* text, Vec2f pos, Vec2f scale = {1, 1}, Vec2f spacing = {5, 8});
void draw_text_buffer(Vec2f pos, Vec2f scale, Vec2f spacing, const char* fmt, ...);


//TODO:
// void draw_text_color(Vec2f pos, Vec2f scale, Vec2f spacing, const char* fmt, ...)
// {
//     char buffer[256];
    
//     va_list args;
//     va_start(args, fmt);
//     vsprintf(buffer, fmt, args); // 'v' = takes a va_list
//     va_end(args);
    
//     draw_text(buffer, pos, scale, spacing);
// }



//DEPRECATED:
// void draw_text_old(const char* text, Vec2f pos, Vec2f scale = {1, 1}, Vec2 spacing = {5, 8})
