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


struct Player
{
    Rect bbox = { .pos = {-2, -7}, .size = {4, Tile(1)-1} };

    f32 ledge_xmargin = 1.5f;
    float32 jump_spd = 1.2f;
    float32 terminal_velocity = 4;

    i32 hurt_state_time = 30;
    i32 hurt_buffer_time = 70;
    
    //                        maxspd, accel, decel, turn, grav
    Physics ground_physics    = {1,  0.03f,  0.045f,  0.025f,  0.1f};
    Physics jump_physics      = {1,  0.02f,  0,      0.02f,  0.06f};
    Physics fall_physics      = {1,  0.015f,  0,      0.03f,  0.08f};
    Physics fall_physics_slow = {0.25f,  0.055f,  0,    0.44f,  0.075f};
    Physics debug_physics     = {2,  0.1f,  0.2f,   0.1f,  0.06f};
    Physics physics;
    // // ground_physics = {1, 0.03f,  0.045f, 0.1f,  0.1f}; //old
    // jump_physics =   {1, 0.025f,  0,      0.1f,  0.06f};
    // fall_physics =   {1, 0.015f,  0,      0.1f,  0.08f};
    // debug_physics =  {2, 0.1f,  0.1f,   0.1f,  0.06f};

    Vec2fUnion(pos, x, y);
    Vec2f spd;
    Vec2f move_input;
    Color color;

    //anim
    Sprite* sprite;
    Vec2f scale = {1, 1};
    float32 anim_index;
    float32 anim_speed;

    Player_State state;
    i32 state_timer;
    b32 hurt_buffer_enabled;
    i32 hurt_buffer_tick;
    f32 ledge_xtarget;
    
    //functions
    void Create(Vec2f pos);
    void Update(Game_Input_Map* input);
    void Draw();

    void move_vert();
    void move_hori(bool32 is_airborne);
    V2f ledge_check_pos(i32* aimdir_return_var = nullptr, i32 aimdir_override = 0, f32 ledge_xmargin_override = 0);
    bool32 ledge_check();

    void state_enter_default(Sprite* _sprite, float32 _anim_index = 0, float32 _anim_speed = 1);
    void state_switch(Player_State new_state);
    void state_perform(Player_State _state, State_Function _function);
};
