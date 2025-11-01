/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */

#define ENT_NAME(type) ENT_INFO[(i32)type].name
#define ENT_NUM(type) pointers->entity->nums[(i32)type]
#define ENT_POINT(type) pointers->entity->pointers[(i32)type]
#define sprite_set(__entity, __sprite_name) __entity->sprite = &pointers->sprite->##__sprite_name


//
enum class Ent_Type
{
    Null = -1,
    
    Player,
    Wall,
    Enemy,
    Spike,
    
    Num,
    All,
};

struct Ent_Info {
    const char* name;
    int max_count;
};

constexpr Ent_Info ENT_INFO[] = {
    { "player", 0 },
    { "wall",   8 },
    { "enemy",  6 },
    { "spike",  2 },
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
    Enter = 0,
    Create = 0,
    
    Leave = 1,
    
    Update = 2,
    Step = 2,

    Draw = 3,
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
    Color color;
    Sprite* sprite;
    float32 anim_index;
    float32 anim_speed = 1;
    Vec2f   scale = {1, 1};

    //bbox
    Vec2f size;

    //movement
    Vec2f move_input;
    Vec2f pos;
    Vec2f spd;

    //combat
    float32 hp;
};
