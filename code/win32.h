/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */

#define BGMODE_TRANSPARENCY_AMT 140



int64 global_cpu_freq;

globalvar BOOL global_use_dark_mode = true;

globalvar Game_Settings* global_settings;
globalvar Game_Input_Map* global_game_input_map;
globalvar bool32 global_bgmode_enabled = false;
globalvar bool32 global_bgmode_trans_when_outoffocus = true;
globalvar bool32 global_running = true;

globalvar bool32 dll_flip; //HACK: for game_dll auto hotloading
//BUG: something is preventing the game dll from loading in (majority of the time) without this
//Upon game dll compilation, we load it into the game by checking its create time vs our stored time.
//For some reason it still gets loaded twice (even with this hack), however-
//the 1 frame delay that this bool creates, HELPS prevent it from failing (for whatever reason)



struct Win32_Render_Buffer
{
    BITMAPINFO info;
    void* memory;
    int pitch;
    int width;
    int height;
    int bytes_per_pixel;
    int memory_size_bytes;
};

struct Win32_Sound_Data
{
    int sample_rate;
    int bytes_per_sample;
    uint32 running_sample_index;
    DWORD buffer_size;
    float32 sin_t;
    int latency_sample_count;
};

struct Win32_Game_Code
{
    //files
    HMODULE game_dll;
    FILETIME game_dll_last_write_time;

    //functions | REQUIRED: 0 when stubbed, always check that function pointers are truthy before calling
    Game_Update_And_Draw* update_and_draw;
    Game_Input_Change_Device* input_change_device;

    bool32 is_valid;
};

struct Win32_Key_Data //NOTE: exists purely to be called with win32_key_check()
{
    MSG message;
    bool32 is_down;
    bool32 was_down;
};

struct Win32_Sleep_Data
{
    float64 estimate;
    float64 mean;
    float64 m2;
    int64   count;
};

struct Win32_Data_Pointers
{
    Win32_Render_Buffer render;
    Win32_Game_Code* game_code;
    Win32_Sleep_Data* sleep;
};

// //TODO: we will need this later
// struct Win32_State
// {
//     bool32 is_bgmode_enabled;
//     bool32 is_bgmode_transparent_out_of_focus;
// };







//XINPUT //NOTE: all this crap is so we dont instacrash for incompatibility
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD, XINPUT_STATE*)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub){ return ERROR_DEVICE_NOT_CONNECTED; }
globalvar x_input_get_state* XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD, XINPUT_VIBRATION*)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub){ return ERROR_DEVICE_NOT_CONNECTED; }
globalvar x_input_set_state* XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_
