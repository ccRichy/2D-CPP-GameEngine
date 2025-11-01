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
inline bool32
collide_pixel_rect(Vec2f pos, Rectangle rect)
{
    return collide_pixel_rect(pos, rect.pos, rect.size);
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



//override
Entity*
collide_pixel_entity_pointer(Vec2f pixel_pos, Ent_Type type)
{
    Entity* result = nullptr;
    Entity* array = ( (type == Ent_Type::All) ? GENTITY->array : ENT_POINT(type) );
    i32 loop_amt = ( (type == Ent_Type::All) ? ENT_MAX_ALL() : ENT_MAX(type) );
    for (int i = 0; i < loop_amt; ++i){
        Entity* ent = &array[i];
        if (!ent->is_alive) continue;
        Rect bb = ent->bbox;
        if (collide_pixel_rect(pixel_pos, ent->pos + bb.pos, bb.size)){
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
    for (int i = 0; i < ENT_MAX_ALL(); ++i){
        Entity* ent = &entity_array[i];
        Rect bb = ent->bbox;
        if (collide_pixel_rect(pixel_pos, ent->pos + bb.pos, bb.size)){
            result = ent;
            break;
        }
    }
    return result;
}


bool32 collide_rect_entity(Vec2f pos, Vec2f size, Ent_Type type)
{
    b32 result = false;
    Entity* array = ( (type == Ent_Type::All) ? ENT_POINT( (Ent_Type)0 ) : ENT_POINT(type) ); 
    i32 loop_amt = ( (type == Ent_Type::All) ? ENT_MAX_ALL() : ENT_MAX(type));
    for (int i = 0; i < loop_amt; ++i){
        Entity* ent = &array[i];
        Rect bb = ent->bbox;
        if (collide_rects(pos, size, ent->pos + bb.pos, bb.size)){
            result = true;
            break;
        }
    }
    return result;
    
}

Entity*
collide_rect_entity_pointer(Vec2f pos, Vec2f size, Ent_Type type)
{
    Entity* result = nullptr;
    Entity* array = ( (type == Ent_Type::All) ? ENT_POINT( (Ent_Type)0 ) : ENT_POINT(type) ); 
    i32 loop_amt = ( (type == Ent_Type::All) ? ENT_MAX_ALL() : ENT_MAX(type));
    for (int i = 0; i < loop_amt; ++i){
        Entity* ent = &array[i];
        Rect bb = ent->bbox;
        if (collide_rects(pos, size, ent->pos + bb.pos, bb.size)){
            result = ent;
            break;
        }
    }
    return result;
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
    // Entity* wall = collide_rect_get_entity(*pos, size, ENT_POINT(Ent_Type::Wall), ENT_MAX(Ent_Type::Wall));
    Entity* wall = collide_rect_entity_pointer(*pos, size, Ent_Type::Wall);
    if (wall)
    {
        Rect wbb = wall->bbox;
        wall_bbox_top = wall->pos.y + wbb.pos.y;
        wall_bbox_bottom = wall_bbox_top + wbb.size.y;

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
    // wall = collide_rect_get_entity(*pos, size, ENT_POINT(Ent_Type::Wall), ENT_MAX(Ent_Type::Wall));
    wall = collide_rect_entity_pointer(*pos, size, Ent_Type::Wall);
    if (wall)
    {
        Rect wbb = wall->bbox;
        wall_bbox_left = wall->pos.x + wbb.pos.x;
        wall_bbox_right = wall_bbox_left + wbb.size.w;

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
