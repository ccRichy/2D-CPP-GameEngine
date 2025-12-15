/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */

//name, max_amount, sprite default
#define ENT_LIST                 \
XMAC(Player, 0, sPlayer_idle)    \
XMAC(Wall,   8, sWall_anim)      \
XMAC(Spike,  2, sSpike)          \
XMAC(Enemy,  6, sNull)           \
XMAC(Turtle, 4, sTurtle_walk)         \
XMAC(Goal,   1, sGoal)           \
XMAC(Orb,    4, sItem_orb)       \



#define ENT_NAME(type)  ENT_INFO[(i32)type].name
#define ENT_NUM(type)   pointers->entity->nums[(i32)type]
#define ENT_POINT(type) pointers->entity->pointers[(i32)type]

#define sprite_set(__entity, __sprite_name) if (pointers->sprite->##__sprite_name.is_initialized) __entity->sprite = &pointers->sprite->##__sprite_name; else __entity->sprite = &pointers->sprite->sDebug

#define bbox_top(__entity)    (__entity->y + __entity->bbox.y)
#define bbox_bottom(__entity) (__entity->y + __entity->bbox.y + __entity->bbox.h)
#define bbox_left(__entity)   (__entity->x + __entity->bbox.x)
#define bbox_right(__entity)  (__entity->x + __entity->bbox.x + __entity->bbox.w)



struct Timer
{
    float32 tick;
    float32 length;
    float32 interval = 1;
    
    bool32 is_active;
    bool32 has_ended;
    
    void start(f32 _length = 0, f32 _interval = 0){
        if (_length > 0)   length = _length;
        if (_interval > 0) interval = _interval;        
        is_active = true;
    }
    void stop(){
        is_active = false;
        has_ended = false;
        tick = 0;
    }
    f32 remaining(){
        f32 result = length - tick;
        return result;
    }
    
    void update(){
        has_ended = false;

        if (!is_active) return;
        
        tick += interval;
        if (tick >= length){
            stop();
            has_ended = true;
        }
    }
};



//
enum class Ent_Type
{
    Null = -1,
    
#define XMAC(__name, ...) __name,
    ENT_LIST
#undef XMAC

    Num,
    All,
};

struct Ent_Info {
    const char* name;
    int max_count;
};

constexpr Ent_Info ENT_INFO[] = {
#define XMAC(__name, __max_count, __sprite) { #__name, __max_count },
    ENT_LIST
#undef XMAC
};

constexpr i32 ENT_MAX_ALL(){
    i32 result = 0;
    for (i32 i = 0; i < (i32)Ent_Type::Num; ++i){
        result += ENT_INFO[i].max_count;
    }
    return result;
}

constexpr i32 ENT_MAX(Ent_Type type){
    i32 result = ENT_INFO[(i32)type].max_count;
    return result;
}



//
enum struct State_Function {
    Enter,
    Create,
    
    Leave,
    
    Step,

    Draw,
};


struct Physics
{
    float32 maxspd;
    float32 accel;
    float32 decel;
    float32 turn;
    float32 grav;
};



//
struct Entity
{
    //base
    Ent_Type type;
    bool32   is_alive;

    //visual
    Color   color;
    Sprite* sprite;
    float32 anim_index;
    float32 anim_speed = 1;
    bool32  anim_ended_this_frame;
    Vec2f   scale      = {1, 1};

    //bbox
    Rectangle bbox; //.pos is treated as an offset
    // Vec2f size;

    //movement
    Vec2fUnion(pos, x, y);
    // Vec2f pos;
    Vec2f move_input;
    Vec2f spd;

    //combat
    float32 hp;
};
