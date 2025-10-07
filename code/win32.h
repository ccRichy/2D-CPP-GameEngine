#pragma once



#define BGMODE_TRANSPARENCY_AMT 100



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




enum struct Win32_Init_Error
{
    SUCCESS_NO_ERROR,
    XINPUT_FAILED_TO_INIT,
    WINDOW_FAILED_TO_REGISTER,
    WINDOW_FAILED_TO_CREATE,
};


struct Win32_Client_Dimensions
{
    int width;
    int height;
};

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

    //functions | REQUIRED: 0 when stubbed, always check != 0 when calling
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



//EXTRA

// struct PNG_Data
// {
//     int32 width;
//     int32 height;
//     int32 bit_depth;
//     int32 color_type;
//     int32 filter_method;
//     int32 interlace_method;
// };
