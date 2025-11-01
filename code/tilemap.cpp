/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */

void
tilemap_clear_all(Tilemap* tmap)
{
    i32 length = tmap->grid_w * tmap->grid_h;
    Tile* tiles = (i32*)tmap->grid;
    for (int i = 0; i < length; ++i){
        tiles[i] = 0;
    }
}

inline int32
tilemap_get_grid_x(Tilemap* tmap, float32 x)
{
    x -= tmap->pos.x;
    int32 result = floor_i32(x / tmap->tile_w);
    return result;
}
inline int32
tilemap_get_grid_y(Tilemap* tmap, float32 y)
{
    y -= tmap->pos.y;
    int32 result = floor_i32(y / tmap->tile_h);
    return result;
}
inline Vec2i
tilemap_get_grid_pos(Tilemap* tmap, Vec2f pos)
{
    pos -= tmap->pos;
    V2i result = {
        floor_i32(pos.x / tmap->tile_w),
        floor_i32(pos.y / tmap->tile_h)
    };
    return result;
}
// inline Vec2f
// tilemap_pos_grid2world(Tilemap* tmap, V2i tile_pos)
// {
//     result = {
//         (f32)(i32)(mouse_pos_world.x / TILE_SIZE) * TILE_SIZE,
//         (f32)(i32)(mouse_pos_world.y / TILE_SIZE) * TILE_SIZE,
//     };
//     return result;
// }


inline bool32
tilemap_grid_pos_within_bounds(Tilemap* tmap, Vec2i grid_pos)
{
    bool32 result = (grid_pos.y >= 0 &&
                     grid_pos.y < TILEMAP_H &&
                     grid_pos.x >= 0 &&
                     grid_pos.x < TILEMAP_W);
    return result;
}
inline bool32
tilemap_world_pos_within_bounds(Tilemap* tmap, Vec2f world_pos)
{
    V2i grid_pos = tilemap_get_grid_pos(tmap, world_pos);
    bool32 result = tilemap_grid_pos_within_bounds(tmap, grid_pos);
    return result;
}

inline Tile*
tilemap_get_tile_at_world_pos(Tilemap* tmap, Vec2f pos)
{
    int32* result = 0;
    V2i tile_pos = tilemap_get_grid_pos(tmap, pos);
    result = &tmap->grid[tile_pos.y][tile_pos.x];
    return result;
}
inline void
tilemap_set_tile_at_world_pos(Tilemap* tmap,  Vec2f pos, int32 value)
{
    if (tilemap_world_pos_within_bounds(tmap, pos))
    {
        int32* tile = tilemap_get_tile_at_world_pos(tmap, pos);
        *tile = value;
    }
}

Rectangle
tilemap_get_tile_rect(Tilemap* tmap, V2i tile_pos)
{
    Rect result;
    result.pos = v2i_to_v2f(tilemap_get_grid_pos(tmap, v2i_to_v2f(tile_pos)));
    result.size = {(f32)tmap->tile_w, (f32)tmap->tile_h};
    return {};
}

void
draw_tilemap(Tilemap* tmap)
{
    f32 zoom             = GSETTING->zoom_scale;
    V2f cam_size         = {BASE_W / zoom, BASE_H / zoom};
    V2f cam_top_left     = pointers->data->camera_pos;
    V2f cam_bottom_right = cam_top_left + cam_size;
    V2i top_left         = tilemap_get_grid_pos(tmap, cam_top_left);
    V2i bottom_right     = tilemap_get_grid_pos(tmap, cam_bottom_right);
    
    bottom_right += {1, 1};
    if (top_left.x < 0) top_left.x = 0;
    if (top_left.y < 0) top_left.y = 0;
    if (bottom_right.x > tmap->grid_w) bottom_right.x = tmap->grid_w;
    if (bottom_right.y > tmap->grid_h) bottom_right.y = tmap->grid_h;
    
    V2f tile_size = v2i_to_v2f({tmap->tile_w, tmap->tile_h});
    for (int Y = top_left.y; Y < bottom_right.y; ++Y){
        for (int X = top_left.x; X < bottom_right.x; ++X){
            if (tmap->grid[Y][X]){
                draw_rect(
                    {(f32)X * (tmap->tile_w), (f32)Y * (tmap->tile_h)},
                    tile_size,
                    BEIGE
                );                
            }
        }
    }
}

bool32
collide_rect_tilemap(Vec2f pos, Vec2f size, Tilemap* tmap)
{
    bool32 result = false;

    float32 bbox_top = pos.y;
    float32 bbox_bottom = bbox_top + size.y;
    float32 bbox_left = pos.x;
    float32 bbox_right = bbox_left + size.x;
        
    int32 grid_pos_top = tilemap_get_grid_y(tmap, bbox_top);
    int32 grid_pos_bottom = tilemap_get_grid_y(tmap, bbox_bottom);
    int32 grid_pos_left = tilemap_get_grid_x(tmap, bbox_left);
    int32 grid_pos_right = tilemap_get_grid_x(tmap, bbox_right);
    
    if (grid_pos_top < 0) grid_pos_top = 0;
    if (grid_pos_bottom >= tmap->grid_h) grid_pos_bottom = tmap->grid_h - 1;
    if (grid_pos_left < 0) grid_pos_left = 0;
    if (grid_pos_right >= tmap->grid_w) grid_pos_right = tmap->grid_w - 1;
    
    for (int Y = grid_pos_top; Y <= grid_pos_bottom; ++Y)
    {
        for (int X = grid_pos_left; X <= grid_pos_right; ++X)
        {
            int32* tile = &tmap->grid[Y][X];
            if (*tile){
                V2f grid_pos_pos = tmap->pos + V2f{(f32)X * TILE_SIZE, (f32)Y * TILE_SIZE};
                if (collide_rects(pos, size, grid_pos_pos, {8, 8})){
                    result = true;
                    break;
                }
            }
        }
    }
    return result;
}
Vec2i 
collide_rect_tilemap_pos(Vec2f pos, Vec2f size, Tilemap* tmap)
{
    Vec2i result = {-1, -1};

    f32 bbox_top = pos.y;
    f32 bbox_bottom = bbox_top + size.y;
    f32 bbox_left = pos.x;
    f32 bbox_right = bbox_left + size.x;
        
    i32 grid_pos_top = tilemap_get_grid_y(tmap, bbox_top);
    i32 grid_pos_bottom = tilemap_get_grid_y(tmap, bbox_bottom);
    i32 grid_pos_left = tilemap_get_grid_x(tmap, bbox_left);
    i32 grid_pos_right = tilemap_get_grid_x(tmap, bbox_right);

    if (grid_pos_top < 0) grid_pos_top = 0;
    if (grid_pos_bottom >= tmap->grid_h) grid_pos_bottom = tmap->grid_h - 1;
    if (grid_pos_left < 0) grid_pos_left = 0;
    if (grid_pos_right >= tmap->grid_w) grid_pos_right = tmap->grid_w - 1;
    
    //HACK: maybe theres a better way to handle this?
    for (i32 Y = grid_pos_top; Y <= grid_pos_bottom; ++Y){
        for (i32 X = grid_pos_left; X <= grid_pos_right; ++X){
            i32* tile = &tmap->grid[Y][X];
            if (*tile){
                V2f grid_pos_pos = tmap->pos + V2f{(f32)X * TILE_SIZE, (f32)Y * TILE_SIZE};
                if (collide_rects(pos, size, grid_pos_pos, {8, 8})){
                    result = {X, Y};
                    break;
                }
            }
        }
    }
    return result;
}

//TILE
Collide_Data
move_collide_tile(Tilemap* tmap, Vec2f* pos, Vec2f* spd, Vec2f size)
{
    Collide_Data coll_data = {};

    f32 caller_bbox_top;
    f32 caller_bbox_bottom;
    f32 caller_bbox_left;
    f32 caller_bbox_right;

    f32 wall_bbox_top;
    f32 wall_bbox_bottom;
    f32 wall_bbox_left;
    f32 wall_bbox_right;

    //
	pos->y += spd->y;
    caller_bbox_top = pos->y;
    caller_bbox_bottom = caller_bbox_top + size.y;
    //Entity* wall = collide_rect_get_entity(*pos, size, pointers->entity->walls, WALL_MAX);
    V2i tile_pos = collide_rect_tilemap_pos(*pos, size, tmap);
    if (tile_pos.x > -1)
    {
        wall_bbox_top = tmap->pos.y + (tile_pos.y * TILE_SIZE);
        wall_bbox_bottom = wall_bbox_top + TILE_SIZE;

        f32 y_res = pos->y;
        if (spd->y > 0)
            y_res = MIN(y_res, wall_bbox_top - size.y);
        if (spd->y < 0)
            y_res = MAX(y_res, wall_bbox_bottom);

        coll_data.vert = true;
        coll_data.ydir = sign(spd->y);
        pos->y = y_res;
        spd->y = 0;
    }

    //
	pos->x += spd->x;
    caller_bbox_left = pos->x;
    caller_bbox_right = caller_bbox_left + size.x;
    /* wall = collide_rect_get_entity(*pos, size, pointers->entity->walls, WALL_MAX); */
    tile_pos = collide_rect_tilemap_pos(*pos, size, tmap);
    if (tile_pos.x > -1)
    {
        wall_bbox_left = tmap->pos.x + (tile_pos.x * TILE_SIZE);
        wall_bbox_right = wall_bbox_left + TILE_SIZE;

        f32 x_res = pos->x;
        if (spd->x > 0)
            x_res = MIN(x_res, wall_bbox_left - size.x);
        if (spd->x < 0)
            x_res = MAX(x_res, wall_bbox_right);

        coll_data.hori = true;
        coll_data.xdir = sign(spd->x);
        pos->x = round_f32(x_res);
        spd->x = 0;
    }
    
    return coll_data;
}
