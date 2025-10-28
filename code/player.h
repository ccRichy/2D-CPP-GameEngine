/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */
struct Sprite;




enum struct State_Function {
    Enter,
    Leave,
    Step
};

enum struct Player_State
{
    Idle,
    Walk,
    Jump,
    Fall,
    Rope
};
struct State
{
    void (*Enter)();
    void (*Step)(Game_Input_Map input);
};
struct Physics
{
    float32 accel;
    float32 decel;
    float32 turn;
    float32 grav;
};

#define PLAYER_DEFAULT_STATE Player_State::Idle
struct Player
{
    //anim
    Sprite* sprite;
    float32 anim_index;
    float32 anim_speed = 1;

    Vec2f pos;
    Vec2f move_input;
    Vec2f spd;
    Vec2f size  = {4, Tile(1)-1};
    Vec2f scale = {1, 1};
    Color color = GREEN;
    
    //numbas
    Physics ground_physics = {0.07f,  0.045f, 0.1f,  0.1f};
    Physics jump_physics =   {0.05f,  0,      0.1f,  0.06f};
    Physics fall_physics =   {0.01f,  0,      0.1f,  0.08f};
    Physics physics; //could become a pointer if it gets too complex?
    
    float32 ground_speed_max = 1;
    float32 jump_spd = 1.2f;
    float32 terminal_velocity = 4;
    int32 aim_dir;
    
    Player_State state = PLAYER_DEFAULT_STATE;
    State_Function state_function; //do we need this?

    void state_switch(Player_State new_state);
    void state_perform(Player_State _state, State_Function _function);

    void Create(Vec2f pos, Player_State _state = PLAYER_DEFAULT_STATE);
    void Update(Game_Input_Map* input);
    void Draw();   
};
