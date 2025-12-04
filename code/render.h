/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */

//name, frame_num, fps, origin
#define PLR_SPR_ORIGIN {8.f, 12.f}
#define SPR_LIST \
XMAC(sPlayer_air         , 7, 15, PLR_SPR_ORIGIN) \
XMAC(sPlayer_air_reach   , 2, 0, PLR_SPR_ORIGIN)  \
XMAC(sPlayer_idle        , 2, 0, PLR_SPR_ORIGIN)  \
XMAC(sPlayer_ledge_grab  , 1, 0, PLR_SPR_ORIGIN)  \
XMAC(sPlayer_ledge       , 4, 10, PLR_SPR_ORIGIN) \
XMAC(sPlayer_ledge_reach , 3, 0, PLR_SPR_ORIGIN)  \
XMAC(sPlayer_rope_climb  , 2, 10, PLR_SPR_ORIGIN) \
XMAC(sPlayer_rope_slide  , 2, 10, PLR_SPR_ORIGIN) \
XMAC(sPlayer_splat_slow  , 5, 7, PLR_SPR_ORIGIN)  \
XMAC(sPlayer_splat_swift , 6, 10, PLR_SPR_ORIGIN) \
XMAC(sPlayer_turn        , 1, 6, PLR_SPR_ORIGIN)  \
XMAC(sPlayer_walk        , 4, 10, PLR_SPR_ORIGIN) \
XMAC(sPlayer_walk_reach  , 4, 10, PLR_SPR_ORIGIN) \
XMAC(sPlayer_wire_idle   , 5, 6, PLR_SPR_ORIGIN)  \
XMAC(sPlayer_wire_walk   , 4, 6, PLR_SPR_ORIGIN)  \
XMAC(sPlayer_hurt        , 1, 0, PLR_SPR_ORIGIN)  \
XMAC(sPlayer_bounce      , 1, 0, PLR_SPR_ORIGIN)  \
XMAC(sPlayer_roll        , 6, 15, PLR_SPR_ORIGIN) \
                                                  \
XMAC(sWall_anim          , 4, 6, {})              \
XMAC(sBlob_small         , 4, 6, {})              \
XMAC(sGoal)                                       \
XMAC(sSpike)                                      \
XMAC(sItem_orb)                                   \
                                                  \
XMAC(sBG_test)                                    \
XMAC(sBG_cave1)                                   \
                                                  \
XMAC(sMouse_cursors      , 4, 0, {2, 2})          \
XMAC(sDebug)                                      \


 
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
    union {
        struct { Vec2i size; };
        struct { int32 width, height; };
    };
    int32 frame_width;
    int32 frame_height;    
    float32 fps;
    uint32 frame_num;
    bool32 is_animation;
    b32 is_initialized;
};


Sprite sprite_create(const char* bmp_filename, uint32 frame_num = 1, float32 fps = 0, Vec2f origin = {});

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
inline void draw_sprite_frame(Sprite* sprite, Vec2f pos, Vec2f scale_overall, Vec2i img_drawpos, Vec2i img_drawsize);





// internal void render_weird_gradient(Game_Render_Buffer* buffer, int blue_offset, int green_offset)
