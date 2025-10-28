/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */
struct Collide_Data
{
    bool32 hori;
    bool32 vert;
    int32 xdir;
    int32 ydir;
    Vec2f spd;
};



//GENERIC
bool32
collide_pixel_rect(Vec2f pixel_pos, Vec2f rect_pos, Vec2f rect_size)
{
	float rect_left = rect_pos.x;
	float rect_right = rect_left + rect_size.x;
	float rect_top = rect_pos.y;
	float rect_bottom = rect_top + rect_size.y;

    bool hori = pixel_pos.x >= rect_left && pixel_pos.x < rect_right;
	bool vert = pixel_pos.y >= rect_top && pixel_pos.y < rect_bottom;

	return (vert && hori);
}
bool32
collide_rects(Vec2f pos1, Vec2f size1, Vec2f pos2, Vec2f size2)
{
    float x1 = pos1.x;
	float y1 = pos1.y;
	float bboxLeft1 = x1;
	float bboxRight1 = x1 + size1.x;
	float bboxTop1 = y1;
	float bboxBottom1 = y1 + size1.y;
	
	float x2 = pos2.x;
	float y2 = pos2.y;
	float bboxLeft2 = x2;
	float bboxRight2 = x2 + size2.x;
	float bboxTop2 = y2;
	float bboxBottom2 = y2 + size2.y;

	bool hori = bboxRight1 > bboxLeft2 && bboxLeft1 < bboxRight2;
	bool vert = bboxBottom1 > bboxTop2 && bboxTop1 < bboxBottom2;

	return (vert && hori);
}

inline bool32
collide_pixel_rect(Vec2f pos, Rectangle rect)
{
    return collide_pixel_rect(pos, rect.pos, rect.size);
}
inline bool32
collide_rects(Rectangle rect1, Rectangle rect2)
{
    return collide_rects(rect1.pos, rect1.size, rect2.pos, rect2.size);
}

//MISC
bool32
on_screen(Vec2f pos, Vec2f padding)
{
    return (
        pos.x + padding.x <= BASE_W ||
        pos.x - padding.x >= 0 ||
        pos.y + padding.y <= BASE_H ||
        pos.y - padding.y >= 0
    );
}



// ENTITY //TODO: move to enitity file?
Entity*
collide_pixel_get_entity(Vec2f pixel_pos, Vec2f size, Entity* entity_array, int32 entity_num)
{
    Entity* result = nullptr;
    for (int i = 0; i < entity_num; ++i)
    {
        Entity* ent = &entity_array[i];
        if (collide_pixel_rect(pixel_pos, ent->pos, ent->size)){
            result = ent;
            break;
        }
    }
    return result;
}
Entity*
collide_pixel_get_any_entity(Vec2f pixel_pos)
{
    Entity* result = nullptr;
    Entity* entity_array = pointers->entity->array;
    for (int i = 0; i < ENT_MAX_ALL(); ++i)
    {
        Entity* ent = &entity_array[i];
        if (collide_pixel_rect(pixel_pos, ent->pos, ent->size))
        {
            result = ent;
            break;
        }
    }
    return result;
}


Entity*
collide_rect_get_entity(Vec2f pos, Vec2f size, Entity* entity_array, int32 entity_num)
{
    for (int i = 0; i < entity_num; ++i)
    {
        Entity* ent = &entity_array[i];
        if (collide_rects(pos, size, ent->pos, ent->size))
            return ent;
    }
    return nullptr;
}

Collide_Data
move_collide_wall(Vec2f* pos, Vec2f* spd, Vec2f size)
{
    Collide_Data coll_data = {};

    float32 caller_bbox_top;
    float32 caller_bbox_bottom;
    float32 caller_bbox_left;
    float32 caller_bbox_right;

    float32 wall_bbox_top;
    float32 wall_bbox_bottom;
    float32 wall_bbox_left;
    float32 wall_bbox_right;

    //
	pos->y += spd->y;
    caller_bbox_top = pos->y;
    caller_bbox_bottom = caller_bbox_top + size.y;
    Entity* wall = collide_rect_get_entity(*pos, size, pointers->entity->walls, ENT_MAX(Ent_Type::Wall));
    if (wall)
    {
        wall_bbox_top = wall->pos.y;
        wall_bbox_bottom = wall_bbox_top + wall->size.y;

        float32 y_res = pos->y;
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
    wall = collide_rect_get_entity(*pos, size, pointers->entity->walls, ENT_MAX(Ent_Type::Wall));
    if (wall)
    {
        wall_bbox_left = wall->pos.x;
        wall_bbox_right = wall_bbox_left + wall->size.x;

        float32 x_res = pos->x;
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
