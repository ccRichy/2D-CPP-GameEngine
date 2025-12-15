/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */

#define PLAYER_DEFAULT_STATE Player_State::Idle

enum struct Player_State
{
    Idle,
    Walk,
    Jump,
    Fall,
    Ledge,
    Rope,
    Hurt,
    Roll,
    Splat,
};


//name, max_amount, sprite default
#define ENT_LIST                 \
XMAC(Player, 1, sPlayer_idle)    \
XMAC(Wall,   8, sWall_anim)      \
XMAC(Spike,  2, sSpike)          \
XMAC(Enemy,  6, sNull)           \
XMAC(Turtle, 4, sTurtle_walk)    \
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
    Vec2f scale      = {1, 1};
    f32   anim_speed = 1;
    f32   anim_index;
    b32   anim_ended_this_frame;
    Color color; //TODO: remove or utilize as a color modifier
    Sprite* sprite;

    //bbox
    Rect bbox; //.pos is treated as an offset

    //movement
    Vec2fUnion(pos, x, y);
    V2f move_input;
    V2f spd;

    //combat
    f32 hp;

    Timer coyote_timer = {.length = 5};
    Timer roll_buffer_timer = {.length = 50};

    f32 jump_spd = 1.2f;
    i32 hurt_buffer_time = 70;
    f32 terminal_velocity = 4;
    f32 ledge_xmargin = 1.5f;
    i32 hurt_state_time = 30;
    
    i32 state_timer;
    b32 hurt_buffer_enabled;
    i32 hurt_buffer_tick;
    f32 ledge_xtarget;
    Player_State player_state;
    
    Physics ground_physics    = {1,     0.03f,  0.045f, 0.025f, 0.1f};
    Physics jump_physics      = {1,     0.02f,  0,      0.02f,  0.06f};
    Physics fall_physics      = {1,     0.015f, 0,      0.03f,  0.08f};
    Physics fall_physics_slow = {0.25f, 0.055f, 0,      0.44f,  0.075f};
    Physics debug_physics     = {2,     0.1f,   0.2f,   0.1f,   0.06f};
    Physics physics;
    
    //funcs
    //player
    void player_create(Vec2f pos);
    void player_update();
    void player_draw();
    
    void player_speed_vert();
    void player_speed_hori(bool32 is_airborne);
    
    V2f player_ledge_check_pos(i32* aimdir_return_var = 0, i32 aimdir_override = 0, f32 xmargin_override = 0);
    bool32 player_ledge_check();

    void player_state_switch(Player_State new_state);
    void player_state_perform(Player_State _state, State_Function _function);
};
