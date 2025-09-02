#include "win32.h"
#include "array_functions.h"
#include "game.h"
#include "my_string.h"

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







internal FILETIME
win32_file_get_write_time(const char* filename)
{
    FILETIME time = {};

    WIN32_FIND_DATA find_data;
    HANDLE find_handle = FindFirstFileA(filename, &find_data);
    if (find_handle != INVALID_HANDLE_VALUE)
    {
        time = find_data.ftLastWriteTime;
        FindClose(find_handle);
    }

    return time;
}

internal bool32
win32_exe_path(char* buffer, bool32 include_filename)
{
    bool32 Result = true;
    
    //walks backwards from the path_len to the first '\\' and starts writing into buffer
    TCHAR exe_path[MAX_PATH]; //get full path
    DWORD path_len = GetModuleFileName(0, exe_path, MAX_PATH);
    if (ERROR_INSUFFICIENT_BUFFER == path_len || 0 == path_len)
        Result = false;
        
    char* one_after_last_slash = exe_path;
    bool32 can_start_writing = false;
    for (int char_i = path_len; char_i >= 0; --char_i)
    {
        if (can_start_writing || include_filename)
        {
            buffer[char_i] = exe_path[char_i];
            if (char_i == 0) break;
        }
        else if (exe_path[char_i] == '\\')
        {
            buffer[char_i] = '\\';
            buffer[char_i+1] = 0;
            can_start_writing = true;
        }
    }

    return Result;
}

internal bool32
win32_delete_file_with_wildcard(LPCSTR file_path)
{
    bool32 Result = true;
    
    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFileA(file_path, &fd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        char buffer[256];
        if (win32_exe_path(buffer, false))
        {
            do
            {
                char buffer2[256];
                string_cat(buffer2, buffer, fd.cFileName);
                DeleteFileA(buffer2);
            } while (FindNextFileA(hFind, &fd));
            FindClose(hFind);
        }
        else Result = false;
    }
    else Result = false;

    return Result;
}



internal Win32_Game_Code
win32_load_game_code(char* dll_path, char* dll_temp_path)
{
    Win32_Game_Code result = {};
    result.update_and_draw = Game_Update_And_Draw_Stub;
    result.input_change_device = Game_Input_Change_Device_Stub;

    result.game_dll_last_write_time = win32_file_get_write_time(dll_path);
    CopyFile(dll_path, dll_temp_path, FALSE);
    result.game_dll = LoadLibraryA(dll_temp_path);
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
win32_reload_game_code(Win32_Game_Code& game_code, char* dll_path, char* dll_temp_path)
{
    win32_unload_game_code(&game_code);
    game_code = win32_load_game_code(dll_path, dll_temp_path);
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




internal Win32_Init_Error
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
        return Win32_Init_Error::SUCCESS_NO_ERROR;
    }
    else
        return Win32_Init_Error::XINPUT_FAILED_TO_INIT;
}

float32 win32_xinput_stick_max(SHORT stick_value)
{
    return (stick_value > 0 ? 32767.0f : 32768.0f);
}


internal void
win32_xinput_poll(Win32_Game_Code* game_code, Game_Input_Map* game_input_map)
{
    //CONTROLLER
    for (DWORD ctrl_index = 0; ctrl_index < XUSER_MAX_COUNT; ctrl_index++ )
    {
        XINPUT_STATE ctrl_state;
        ZeroMemory( &ctrl_state, sizeof(XINPUT_STATE) );
                    
        if( XInputGetState( ctrl_index, &ctrl_state ) == ERROR_SUCCESS )
        {
            XINPUT_GAMEPAD* pad = &ctrl_state.Gamepad;

            bool32 ButtonStart = (pad->wButtons & XINPUT_GAMEPAD_START) > 0;
            bool32 ButtonBack =  (pad->wButtons & XINPUT_GAMEPAD_BACK) > 0;

            bool32 dpad_u = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP) > 0;
            bool32 dpad_d = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN) > 0;
            bool32 dpad_l = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT) > 0;
            bool32 dpad_r = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) > 0;

            bool32 a_button = (pad->wButtons & XINPUT_GAMEPAD_A) > 0;
            bool32 b_button = (pad->wButtons & XINPUT_GAMEPAD_B) > 0;
            bool32 x_button = (pad->wButtons & XINPUT_GAMEPAD_X) > 0;
            bool32 y_button = (pad->wButtons & XINPUT_GAMEPAD_Y) > 0;

            bool32 lstick_click = (pad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB) > 0;
            bool32 rstick_click = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) > 0;
                            
            bool32 l_button = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) > 0;
            bool32 r_button = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) > 0;
            int8 l_trig =  pad->bLeftTrigger;
            int8 r_trig =  pad->bRightTrigger;

            SHORT deadzone_l = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
            SHORT deadzone_r = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
                            
            //input device state
            if ((game_input_map->game_input_device != Game_Input_Device::controller) &&
                (pad->wButtons ||
                 abs_i(pad->sThumbLX) > deadzone_l*3 || abs_i(pad->sThumbLY) > deadzone_l*3 ||
                 abs_i(pad->sThumbRX) > deadzone_l*3 || abs_i(pad->sThumbLY) > deadzone_l*3 ||
                 l_trig > 100 || r_trig > 100))
            {
                game_code->input_change_device(Game_Input_Device::controller, game_input_map);
            }

            //send out inputs
            //TODO: abstration layer for rebinding
            if (game_input_map->game_input_device == Game_Input_Device::controller)
            {
                Game_Input_Map* _in = game_input_map;

                float32 lstickx = 0;
                if (abs(pad->sThumbLX) > deadzone_l)
                    lstickx = (float32)pad->sThumbLX;
                if (lstickx > 0)
                    lstickx = map_value(lstickx, { (float32)deadzone_l, 32767 }, {0, 1});
                else if (lstickx < 0)
                    lstickx = map_value(lstickx, { -32768, (float32)-deadzone_l }, {-1, 0});

                float32 lsticky = 0;
                if (abs(pad->sThumbLY) > deadzone_l)
                    lsticky = (float32)pad->sThumbLY;
                if (lsticky > 0)
                    lsticky = map_value(lsticky, { (float32)deadzone_l, 32767 }, {0, 1});
                else if (lsticky < 0)
                    lsticky = map_value(lsticky, { -32768, (float32)-deadzone_l }, {-1, 0});
                //axes
                _in->l_axes = {lstickx, lsticky};
                            
                //buttons
                _in->up.hold = dpad_u;
                _in->down.hold = dpad_d;
                _in->left.hold = dpad_l;
                _in->right.hold = dpad_r;
            }
        }
        else
        {
            // Controller is not connected
            //TODO: handle controller disconnection
        }
    }
    
}


internal void
win32_process_pending_messages(Win32_Game_Code* game_code, Game_Input_Map* game_input_map, bool32* global_is_running)
{
    MSG message;
    while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
    {
        switch(message.message)
        {
            case WM_QUIT:{
                *global_is_running = false;
            }break;
                            
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:{
                if (game_input_map->game_input_device != Game_Input_Device::keyboard_mouse)
                    game_code->input_change_device(Game_Input_Device::keyboard_mouse, game_input_map);
            
                uint32 vk_code = (uint32)message.wParam;

                Game_Input_Map* _in = game_input_map;
                bool32 is_down = ((message.lParam & (1 << 31)) == 0);
                bool32 was_down = ((message.lParam & (1 << 30)) != 0);
            
                if (was_down != is_down)
                {
                    if (vk_code == 'W') _in->up.hold = is_down;
                    else if (vk_code == 'S') _in->down.hold = is_down;
                    else if (vk_code == 'A') _in->left.hold = is_down;
                    else if (vk_code == 'D') _in->right.hold = is_down;
                    //else if (vk_code == VK_SPACE) 
                    //else if (vk_code == VK_CONTROL)
                    //else if (vk_code == VK_SHIFT)
                    //else if (vk_code == VK_MENU) //alt
                    //else if (vk_code == VK_RETURN)
                                    

                    //alt f4
                    bool32 alt_hold = (message.lParam & (1 << 29)) != 0;
                    if (alt_hold && vk_code == VK_F4)
                        *global_is_running = false;
                    if (vk_code == VK_ESCAPE)
                        *global_is_running = false;
                }
            }break;

            default:{
                TranslateMessage(&message);
                DispatchMessageA(&message);                                
            }break;
        }
                    
    }

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
