/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */

#define TILE_SIZE 8
#define TILEMAP_W 1024
#define TILEMAP_H 1024
#define Tile(value) (value * TILE_SIZE)


typedef int32 Tile;
struct Tilemap
{
    Vec2f pos;
    Tile grid[TILEMAP_H][TILEMAP_W];

    union {
        struct {
            V2i grid_size;
            V2i tile_size;
        };
        struct {
            int32 grid_w;
            int32 grid_h;
            int32 tile_w;
            int32 tile_h;
        };
    };
};
