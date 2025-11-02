/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */
enum struct Player_State
{
    Idle,
    Walk,
    Jump,
    Fall,
    Ledge,
    Rope,
    Hurt,
};


#define PLAYER_DEFAULT_STATE Player_State::Idle
struct Player
{
    //anim
    Sprite* sprite;
    float32 anim_index;
    float32 anim_speed = 1;

    Vec2fUnion(pos, x, y);
    Vec2f spd;
    Vec2f move_input;
    Rect bbox = { 0, 0, 4, Tile(1)-1 };
    // Rect bbox = { {}, {4, Tile(1)-1} };
    // Vec2f size  = {4, Tile(1)-1};
    Vec2f scale = {1, 1};
    Color color = GREEN;

    //numbas
    Physics ground_physics = {1, 0.07f,  0.045f, 0.1f,  0.1f};
    Physics jump_physics =   {1, 0.05f,  0,      0.1f,  0.06f};
    Physics fall_physics =   {1, 0.01f,  0,      0.1f,  0.08f};
    Physics debug_physics =  {1, 0.01f,  0.1f,   0.1f,  0.07f};
    Physics physics; //could become a pointer if it gets too complex?

    // float32 ground_speed_max  = 1;
    float32 jump_spd          = 1.2f;
    float32 terminal_velocity = 4;

    void Create(Vec2f pos);
    void Update(Game_Input_Map* input);
    void Draw();

    Player_State state;
    void state_switch(Player_State new_state);
    void state_perform(Player_State _state, State_Function _function);
};
