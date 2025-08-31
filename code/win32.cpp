#include "win32.h"
#include "array_functions.h"
#include "game.h"

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

struct Win32_Game_Code
{
    HMODULE game_dll;
    Game_Update_And_Draw* update_and_draw;
    Game_Input_Change_Device* input_change_device;

    bool32 is_valid;
};


internal Win32_Game_Code
win32_load_game_code()
{
    CopyFile("../build/game.dll", "../build/game_temp.dll", FALSE);
    Win32_Game_Code result = {};
    result.update_and_draw = Game_Update_And_Draw_Stub;
    result.input_change_device = Game_Input_Change_Device_Stub;
    
    result.game_dll = LoadLibraryA("game_temp.dll");
    if (result.game_dll)
    {
        result.update_and_draw = (Game_Update_And_Draw*)GetProcAddress(result.game_dll, "game_update_and_draw");
        result.input_change_device = (Game_Input_Change_Device*)GetProcAddress(result.game_dll, "game_input_change_device");
    }

    if (result.input_change_device && result.update_and_draw)
        result.is_valid = true;

    return result;
}
internal void
win32_unload_game_code(Win32_Game_Code* game_code)
{
    if (game_code->game_dll)
    {
        FreeLibrary(game_code->game_dll);
        game_code->game_dll = 0;
    }

    game_code->is_valid = false;
    game_code->update_and_draw = Game_Update_And_Draw_Stub;
    game_code->input_change_device = Game_Input_Change_Device_Stub;
}

internal void
win32_reload_game_code(Win32_Game_Code& game_code)
{
    win32_unload_game_code(&game_code);
    game_code = win32_load_game_code();
}




#if MY_INTERNAL
void DEBUG_platform_file_free_memory(void* memory)
{
    VirtualFree(memory, 0, MEM_RELEASE);
}

DEBUG_File
DEBUG_platform_file_read_entire(const char* name)
{
    DEBUG_File result = {};
    
    HANDLE handle = CreateFileA(name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (handle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER size;
        if (GetFileSizeEx(handle, &size))
        {
            uint32 size_32 = safe_truncate_uint64(size.QuadPart);
            result.memory = VirtualAlloc(0, size_32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if (result.memory)
            {
                DWORD bytes_read;
                if (ReadFile(handle, result.memory, size_32, &bytes_read, 0) &&
                    (bytes_read == size_32))
                {
                    result.size = bytes_read;
                    //NOTE: success
                }
                else
                {
                    DEBUG_platform_file_free_memory(result.memory);
                    result.memory = 0;
                }
            }
            else
            {
                //TODO: Log - allocation fail
            }
        }
        else
        {
            //TODO: Log - file size failed
        }
        
        CloseHandle(handle);
    }
    else
    {
        
    }

    return result;
}

bool32
DEBUG_platform_file_write_entire(const char* name, uint32 memory_size, void* memory)
{
    bool32 result = false;
    
    HANDLE handle = CreateFileA(name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (handle != INVALID_HANDLE_VALUE)
    {
        DWORD bytes_written;
        if (WriteFile(handle, memory, memory_size, &bytes_written, 0))
        {
            result = (bytes_written == memory_size);
            //NOTE: success
        }
        else
        {
            //TODO: LOG - failed to write
        }
        CloseHandle(handle);
    }
    else
    {
        //NOTE: LOG - invalid handle
    }

    return result;
}
#endif




internal void
win32_load_xinput()
{    
    HMODULE xinput_library = LoadLibrary("xinput1_4.dll");
    if (!xinput_library)
    {
        //TODO: Diagnostic
        xinput_library = LoadLibrary("xinput9_1_0.dll");
    }
    if (!xinput_library)
    {
        //TODO: Diagnostic
        xinput_library = LoadLibrary("xinput1_3.dll");
    }
    
    if (xinput_library)
    {
        XInputGetState = (x_input_get_state*)GetProcAddress(xinput_library, "XInputGetState");
        XInputSetState = (x_input_set_state*)GetProcAddress(xinput_library, "XInputSetState");
        //TODO: Diagnostic
    }
}

float32 win32_xinput_stick_max(SHORT stick_value)
{
    return (stick_value > 0 ? 32767.0f : 32768.0f);
}


internal void
win32_process_input(Game_Input_Map* input_map, Game_Input_Map* input_map_prev)
{
    // input_map_prev.up.hold = true;
    int inputs_num = array_length(input_map->buttons);
    for (int i = 0; i < inputs_num; ++i)
    {
        Game_Input_Button* state_prev = &input_map_prev->buttons[i];
        Game_Input_Button* state = &input_map->buttons[i];
        
        bool32 held = state->hold;
        bool32 held_prev = state_prev->hold;
        
        state->press = !held_prev && held;
        state->release = held_prev && !held;
        
        state_prev->hold = held;
    }
}


//////////////////////////////////
//WINDOW//////////////////////////
internal Win32_Client_Dimensions win32_get_client_dimensions(HWND window)
{
    RECT client_rect;
    GetClientRect(window, &client_rect);
    return {client_rect.right - client_rect.left, client_rect.bottom - client_rect.top};
}


internal void
win32_resize_DIB_section(Win32_Render_Buffer* buffer, int width, int height)
{
    //bm
    if (buffer->memory)
    {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }

    buffer->width = width;
    buffer->height = height;
    int bytes_per_pixel = 4;
    buffer->bytes_per_pixel = bytes_per_pixel;
    
    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = buffer->width;
    buffer->info.bmiHeader.biHeight = -buffer->height;
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB;

    //Draw Pixels loop
    //int bytesPerPixel = 4; //to store RGB + pad //0xRRGGBBxx //global now
    int bitmap_memory_size = (width * height) * bytes_per_pixel;
    buffer->memory = VirtualAlloc(0, bitmap_memory_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

    buffer->pitch = buffer->width * bytes_per_pixel; //the distance between two rows
}


internal void
win32_display_buffer_in_window(Win32_Render_Buffer* buffer, HDC device_context,
                               int window_width, int window_height)
{
    StretchDIBits(device_context,
                  0, 0, window_width, window_height,
                  0, 0, buffer->width, buffer->height,
                  buffer->memory,
                  &buffer->info,
                  DIB_RGB_COLORS, SRCCOPY
    );
}




internal void
win32_debug_draw_vertical_line(Win32_Render_Buffer* render_buffer, int x, int top, int bottom, uint32 color)
{
    uint8* pixel = ((uint8*)render_buffer->memory +
            x * render_buffer->bytes_per_pixel +
            top * render_buffer->pitch);
    
    for (int Y = 0; Y < bottom; ++Y)
    {
        *(uint32*)pixel = color;
        pixel += render_buffer->pitch;
    }
}
