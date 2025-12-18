#include "win32.h"



globalvar Win32_Render_Buffer global_render_buffer;



//
inline float64 win32_tick_to_ms(int64 tick){
    return ((float64)tick * 1000) / global_cpu_freq;
}
inline float64 win32_tick_to_sec(int64 tick){
    return ((float64)tick) / (float64)global_cpu_freq;
}


//perf query
inline int64
win32_get_tick()
{
    LARGE_INTEGER ticks;
    if (!QueryPerformanceCounter(&ticks))
    {
        return 0;
    }
    return ticks.QuadPart;
}

inline int64
win32_get_tick_diff(int64 tick_prev)
{
    return (win32_get_tick() - tick_prev);
}



//function is based on this: https://blat-blatnik.github.io/computerBear/making-accurate-sleep-function/
internal void
win32_sleep_well(double seconds, Win32_Sleep_Data* sleep_data)
{
    if (seconds == 0) return; //NOTE: debug

    while (seconds > sleep_data->estimate)
    {
        auto start = win32_get_tick();
        Sleep(1);
        auto end =   win32_get_tick();
        
        float64 sec_observed  = (double)(end - start) / global_cpu_freq;
        seconds          -= sec_observed;

        sleep_data->count++;
        float64 delta         = sec_observed - sleep_data->mean;
        sleep_data->mean     += delta / sleep_data->count;
        sleep_data->m2       += delta * (sec_observed - sleep_data->mean);
        double stddev        = sqrt_f64(sleep_data->m2 / (sleep_data->count - 1));
        sleep_data->estimate  = sleep_data->mean + stddev;
    }

    // spin lock
    auto start = win32_get_tick();
    auto sec_while = win32_tick_to_ms(win32_get_tick_diff(start)) / 1000;

    while(sec_while < seconds)
    {
        sec_while = win32_tick_to_sec(win32_get_tick_diff(start));
    }
}






//FILES
internal FILETIME
win32_file_get_write_time(const char* filepath)
{
    FILETIME time = {};

#if 0
    WIN32_FIND_DATA find_data;
    HANDLE find_handle = FindFirstFileA(filepath, &find_data);
    if (find_handle != INVALID_HANDLE_VALUE)
    {
        time = find_data.ftLastWriteTime;
        FindClose(find_handle);
    }
#else
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesEx(filepath, GetFileExInfoStandard, &data))
    {
        time = data.ftLastWriteTime;
    }
#endif
    
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
win32_delete_file_wildcard(LPCSTR file_path)
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



///Dynamic  Loading
internal Win32_Game_Code
win32_load_game_code(char* dll_path, char* dll_temp_path)
{
    Win32_Game_Code result = {};
    result.update_and_draw = 0;
    result.input_change_device = 0;

    result.game_dll_last_write_time = win32_file_get_write_time(dll_path);
    CopyFile(dll_path, dll_temp_path, FALSE);
    result.game_dll = LoadLibraryA(dll_temp_path);
    if (result.game_dll)
    {
        result.update_and_draw = (Game_Update_And_Draw*)GetProcAddress(result.game_dll, "game_update_and_draw");
        result.input_change_device = (Game_Input_Change_Device*)GetProcAddress(result.game_dll, "game_input_change_device");
    }
    else
    {
        OutputDebugStringA("(Win32)(ERROR): game dll failed to load\n");
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
    game_code->update_and_draw = 0;
    game_code->input_change_device = 0;
}

internal void
win32_reload_game_code(Win32_Game_Code& game_code, char* dll_path, char* dll_temp_path)
{
    win32_unload_game_code(&game_code);
    game_code = win32_load_game_code(dll_path, dll_temp_path);
}



//INTERNAL
#if MY_INTERNAL
DEBUG_PLATORM_FILE_FREE_MEMORY(DEBUG_platform_file_free_memory)
{
    VirtualFree(memory, 0, MEM_RELEASE);
}

DEBUG_PLATORM_FILE_READ_ENTIRE(DEBUG_platform_file_read_entire)
{
    DEBUG_File result = {};
    
    HANDLE handle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
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
        //invalid handle
    }

    return result;
}


DEBUG_PLATORM_FILE_WRITE_ENTIRE(DEBUG_platform_file_write_entire)
{
    bool32 result = false;
    
    HANDLE handle = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
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



void
level_write_test(const char* filename)
{
    HANDLE handle = CreateFileA(filename, FILE_APPEND_DATA, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (handle == INVALID_HANDLE_VALUE) return;

    const char* test_string = "test\n";
    DWORD bytes_written;
    if (WriteFile(handle, test_string, sizeof(char) * string_length(test_string), &bytes_written, 0))
    {
        //success
    }
    else
    {
        //TODO: LOG - failed to write
    }
    if (WriteFile(handle, test_string, sizeof(char) * string_length(test_string), &bytes_written, 0))
    {
        //success
    }
    else
    {
        //TODO: LOG - failed to write
    }
    CloseHandle(handle);
}

void
level_read_test(const char* filename)
{
    HANDLE handle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (handle == INVALID_HANDLE_VALUE) return;

    LARGE_INTEGER size;
    if (!GetFileSizeEx(handle, &size)) return;
    uint32 size_32 = safe_truncate_uint64(size.QuadPart);
    
    DWORD bytes_read;
    char buffer[256];
    ReadFile(handle, buffer, size_32, &bytes_read, 0);
    
    CloseHandle(handle);
}



//INPUT
internal bool32
win32_load_xinput()
{    
    HMODULE xinput_library = LoadLibrary("xinput1_4.dll");
    if (!xinput_library)
    {
        //TODO: Diagnostic
        xinput_library = LoadLibrary("xinput9_1_0.dll");
        OutputDebugStringA("(XInput): ver. 9_1_0 found\n");
    }
    if (!xinput_library)
    {
        //TODO: Diagnostic
        xinput_library = LoadLibrary("xinput1_3.dll");
        OutputDebugStringA("(XInput): ver. 1_3 found\n");
    }
    else OutputDebugStringA("(XInput): ver. 1_4 found\n");
    
    if (xinput_library)
    {
        XInputGetState = (x_input_get_state*)GetProcAddress(xinput_library, "XInputGetState");
        XInputSetState = (x_input_set_state*)GetProcAddress(xinput_library, "XInputSetState");
        //TODO: Diagnostic
        OutputDebugStringA("(XInput): Loaded Successfully\n");
        return true;
    }
    else
    {
        OutputDebugStringA("(XInput)(ERROR): Failed to load\n");
        return false;
    }
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
            if ((game_input_map->input_device != Game_Input_Device::Controller) &&
                (pad->wButtons ||
                 abs_i32(pad->sThumbLX) > deadzone_l*3 || abs_i32(pad->sThumbLY) > deadzone_l*3 ||
                 abs_i32(pad->sThumbRX) > deadzone_l*3 || abs_i32(pad->sThumbLY) > deadzone_l*3 ||
                 l_trig > 100 || r_trig > 100))
            {
                if (game_code->input_change_device)
                    game_code->input_change_device(Game_Input_Device::Controller, game_input_map);
            }

            //send out inputs
            //TODO: abstration layer for rebinding
            if (game_input_map->input_device == Game_Input_Device::Controller)
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

internal bool32
win32_key_check(WPARAM vk_code, Game_Input_Button& game_input_button, Win32_Key_Data key_data)
{
    if (key_data.message.wParam == vk_code)
    {
        game_input_button.hold = key_data.is_down;
        return true;
    }
    return false;
}

internal void
win32_process_input(Game_Input_Map* input_map, Game_Input_Map* input_map_prev)
{
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



//WINDOW
//TODO: MAYBE split out inputs and others into their own functions. wait til it becomes too much?
internal void
win32_process_pending_messages(Win32_Game_Code* game_code, Game_Input_Map* game_input_map, bool32* global_is_running, Typing_Buffer* typing_buffer)
{
    MSG message;
    Game_Input_Map* in = game_input_map;
    while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
    {
        switch(message.message)
        {
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:{
                //device switch
                if (game_input_map->input_device != Game_Input_Device::Keyboard_Mouse)
                    if (game_code->input_change_device)
                        game_code->input_change_device(Game_Input_Device::Keyboard_Mouse, game_input_map);

                uint32 vk_code = (uint32)message.wParam;
                bool32 is_down = ((message.lParam & (1 << 31)) == 0);
                bool32 was_down = ((message.lParam & (1 << 30)) != 0);

                Win32_Key_Data key_data;
                key_data.message = message;
                key_data.is_down = is_down;
                key_data.was_down = was_down;
                
                if (in->input_mode == Input_Mode::Play)
                {
                    if (was_down != is_down)
                    {                    
                        //game
                        win32_key_check('W',      in->up,    key_data);
                        win32_key_check('S',      in->down,  key_data);
                        win32_key_check('A',      in->left,  key_data);
                        win32_key_check('D',      in->right, key_data);
                        win32_key_check(VK_SPACE, in->jump,  key_data);
                        win32_key_check(VK_SHIFT, in->shoot, key_data);

                        //editor
                        win32_key_check(VK_F12,    in->edit_toggle,     key_data);
                        win32_key_check(VK_DELETE, in->edit_delete,     key_data);
                        win32_key_check(VK_OEM_2,  in->edit_save,     key_data);
                        win32_key_check(VK_OEM_5,  in->edit_load,     key_data);

                        //debug
                        win32_key_check('R',          in->reset,               key_data);
                        win32_key_check(VK_F1,        in->debug_mode_toggle,   key_data);
                        win32_key_check(VK_F8,        in->debug_bgmode_toggle, key_data);
                        win32_key_check(VK_OEM_PLUS,  in->debug_win_plus,      key_data);
                        win32_key_check(VK_OEM_MINUS, in->debug_win_minus,     key_data);
                        win32_key_check('W', in->debug_win_setup,     key_data);

                        win32_key_check(VK_F5, in->debug_hotkey1,  key_data);
                        win32_key_check(VK_F6, in->debug_hotkey2,  key_data);
                        win32_key_check(VK_F7, in->debug_hotkey3,  key_data);

                        //misc
                        win32_key_check(VK_CONTROL, in->ctrl,    key_data);
                        win32_key_check(VK_SHIFT,   in->shift,   key_data);
                        win32_key_check(VK_MENU,    in->alt,     key_data);
                        win32_key_check(VK_SPACE,   in->space,   key_data);
                        win32_key_check(VK_RETURN,  in->enter,   key_data);
                        win32_key_check(VK_ESCAPE,  in->escape,  key_data);
                        win32_key_check(VK_OEM_3,   in->console, key_data);

                        win32_key_check('0', in->num0,  key_data);
                        win32_key_check('1', in->num1,  key_data);
                        win32_key_check('2', in->num2,  key_data);
                        win32_key_check('3', in->num3,  key_data);
                        win32_key_check('4', in->num4,  key_data);
                        win32_key_check('5', in->num5,  key_data);
                        win32_key_check('6', in->num6,  key_data);
                        win32_key_check('7', in->num7,  key_data);
                        win32_key_check('8', in->num8,  key_data);
                        win32_key_check('9', in->num9,  key_data);
                    }
                }
                else if (in->input_mode == Input_Mode::Type)
                {
                    BYTE key_state[256];
                    GetKeyboardState(key_state);
                    WORD ascii_out[2];
                    int ta_result = ToAscii(vk_code, (message.lParam >> 16) & 0xFF,
                                            key_state, ascii_out, 0);

                    if (was_down != is_down)
                    {
                        win32_key_check(VK_CONTROL, in->ctrl,    key_data);
                        win32_key_check(VK_RETURN,  in->enter,   key_data);
                        win32_key_check(VK_ESCAPE,  in->escape,  key_data);
                        win32_key_check(VK_OEM_3,   in->console, key_data);                        
                    }
                    if (is_down)
                    {
                        //HACK: we are handling special keys explicitly for typing mode, but we might want to be more deliberate about this in the future
                        if (ta_result > 0)
                        { 
                            u8 key = (u8)ascii_out[0];
                            if (key == '\b' && typing_buffer->length > 0) //backspace
                            {
                                typing_buffer->length--;
                                typing_buffer->items[typing_buffer->length] = 0;
                            }
                            else if (typing_buffer->length < TYPING_BUFFER_SIZE)
                            {
                                // if (vk_code == VK_RETURN) key = '\n';
                                if (is_char_type(key, Char_Type::Printable))
                                {
                                    typing_buffer->items[typing_buffer->length] = key;
                                    typing_buffer->length++;
                                }
                            }
                        }
                    }
                }

                bool32 alt_hold = (message.lParam & (1 << 29)) != 0;
                //Quitting
                if (is_down)
                {
                    if (alt_hold && vk_code == 'Q')
                        *global_is_running = false;
                }

            }break;

            case WM_MOUSEWHEEL:{
                int32 zDelta = GET_WHEEL_DELTA_WPARAM(message.wParam);
                in->mouse_scroll = sign(zDelta);
            }break;
                
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
            case WM_XBUTTONDOWN:
            case WM_XBUTTONUP:{
                UINT mbs = message.wParam & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON | MK_XBUTTON1 | MK_XBUTTON2);
                
                in->mouse_left.hold   = (mbs & MK_LBUTTON) != 0;
                in->mouse_right.hold  = (mbs & MK_RBUTTON) != 0;
                in->mouse_middle.hold = (mbs & MK_MBUTTON) != 0;
                in->mouse_back.hold   = (mbs & MK_XBUTTON1) != 0;
                in->mouse_front.hold  = (mbs & MK_XBUTTON2) != 0;
            }break;
            case WM_MOUSEMOVE:{
                POINTS mouse_points = POINTS MAKEPOINTS(message.lParam);
                //NOTE: pixel aligned
                in->mouse_pos_gui = {
                    round_f32(mouse_points.x / global_settings->render_scale),
                    round_f32(mouse_points.y / global_settings->render_scale)
                };
                
                // //NOTE: float
                // in->mouse_pos = {
                //     (float32)(mouse_points.x / Global_Settings->window_scale),
                //     (float32)(mouse_points.y / Global_Settings->window_scale)
                // };
            }break;

                
            default:{
                TranslateMessage(&message);
                DispatchMessageA(&message);                                
            }break;
        }
                    
    }

}


internal void
win32_set_DIB(Win32_Render_Buffer* buffer, int width, int height)
{
    //bm
    if (buffer->memory)
        VirtualFree(buffer->memory, 0, MEM_RELEASE);

    i32 bytes_per_pixel = 4;
    buffer->width = width;
    buffer->height = height;
    buffer->bytes_per_pixel = bytes_per_pixel;
    buffer->memory_size_bytes = width * height * bytes_per_pixel;
    buffer->pitch = buffer->width * bytes_per_pixel; //bytes per row
    
    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = buffer->width;
    buffer->info.bmiHeader.biHeight = -buffer->height;
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB;

    buffer->memory = VirtualAlloc(0, buffer->memory_size_bytes, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
}


internal void
win32_display_buffer_in_window(Win32_Render_Buffer* buffer, HDC device_context,
                               i32 window_width, i32 window_height)
{
    StretchDIBits(device_context,
                  0, 0, window_width, window_height, //dest
                  0, 0, buffer->width, buffer->height, //src
                  buffer->memory,
                  &buffer->info,
                  DIB_RGB_COLORS, SRCCOPY
    );
}



internal V2i
win32_get_client_dimensions(HWND window)
{
    RECT client_rect;
    GetClientRect(window, &client_rect);
    return {client_rect.right - client_rect.left, client_rect.bottom - client_rect.top};
}

internal V2i
win32_get_monitor_resolution(HWND window)//, int* width, int* height)
{
    HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);
    MONITORINFOEX info = {};
    info.cbSize = sizeof(MONITORINFOEX);
    BOOL info_result = GetMonitorInfo(monitor, &info);
    DEVMODE devmode = {};
    devmode.dmSize = sizeof(DEVMODE);
    BOOL disp_settings_result = EnumDisplaySettings(info.szDevice, ENUM_CURRENT_SETTINGS, &devmode);

    return {(i32)devmode.dmPelsWidth, (i32)devmode.dmPelsHeight};
    // *width = devmode.dmPelsWidth;
    // *height = devmode.dmPelsHeight;
}

internal V2i
window_get_size(HWND window, f32 scale_override = -1)
{
    RECT window_rect;
    if (scale_override == -1){
        GetWindowRect(window, &window_rect);
    }
    else{
        window_rect = {0, 0, (i32)(BASE_W * scale_override), (i32)(BASE_H * scale_override)};
        DWORD curr_style = GetWindowLong(window, GWL_STYLE);
        AdjustWindowRectEx(&window_rect, curr_style, FALSE, 0);
    }
    
    return {window_rect.right - window_rect.left, window_rect.bottom - window_rect.top};
}



internal void
window_set_trans(HWND window, bool32 enabled)
{
    if (enabled){
        BOOL win_set_attrib_result = SetLayeredWindowAttributes(window, 0, BGMODE_TRANSPARENCY_AMT, LWA_ALPHA);        
    }
    else{
        BOOL win_set_attrib_result = SetLayeredWindowAttributes(window, 0, 255, LWA_ALPHA);
    }    
}

internal void
window_set_topmost(HWND window, bool32 enabled)
{
    RECT window_rect = {0, 0, (int32)(BASE_W * global_settings->window_scale), (int32)(BASE_H * global_settings->window_scale)};
    DWORD curr_style = GetWindowLong(window, GWL_STYLE);
    AdjustWindowRectEx(&window_rect, curr_style, FALSE, 0);
    RECT same_pos;
    GetWindowRect(window, &same_pos);
    HWND z_order = (enabled ? HWND_TOPMOST : HWND_NOTOPMOST);
    BOOL win_pos_result = SetWindowPos(
        window, z_order,
        same_pos.left, same_pos.top, //xy
        window_rect.right - window_rect.left, window_rect.bottom - window_rect.top,
        SWP_SHOWWINDOW
    );
}

internal void
window_center(HWND window)
{
    V2i res = win32_get_monitor_resolution(window);//, &disp_w, &disp_h);
    V2i winsize = window_get_size(window);
    int32 target_x = (res.x/2) - (winsize.x/2);
    int32 target_y = (res.y/2) - (winsize.y/2);
    
    SetWindowPos(
        window, 0,
        target_x, target_y,
        0, 0,
        SWP_NOSIZE|SWP_SHOWWINDOW
    );

}

internal void
window_set_scale(float32 scale, HWND window, Win32_Render_Buffer* win32_render_buffer, Game_Render_Buffer* game_render_buffer)
{
    global_settings->window_scale = scale;
    V2i winsize = window_get_size(window, scale);
    //TODO: handle changing the render scale differently
    // win32_set_DIB(win32_render_buffer, (i32)(BASE_W * scale), (i32)(BASE_H * scale));
    SetWindowPos(
        window, 0,
        0, 0,
        winsize.x, winsize.y,
        SWP_NOMOVE|SWP_SHOWWINDOW
    );
    window_center(window);
    
    game_render_buffer->memory = win32_render_buffer->memory;
    game_render_buffer->width  = win32_render_buffer->width;
    game_render_buffer->height = win32_render_buffer->height;
    game_render_buffer->pitch  = win32_render_buffer->pitch;
}

internal void
window_set_pos(HWND window, i32 x, i32 y)
{
    SetWindowPos(
        window, 0,
        x, y,
        0, 0,
        SWP_NOSIZE|SWP_SHOWWINDOW
    );
}



///EXTRA
// void PNG_Load(const char* filename)
// {
//     DEBUG_File file = DEBUG_platform_file_read_entire(filename);
    
//     uint64* file_32 = (uint64*)file.memory; //full signature
//     file_32++; //skip to next 8 bytes;
//     for (uint32 i = 0; i < file.size; ++i)
//     {
//         uint8* file_8 = (uint8*)file_32;
//         file_32++;
//     }
//     uint64 type = 0x89504e470d0a1a0a; //TODO: typecheck //this is probably wrong
// }


// internal void
// win32_debug_draw_vertical_line(Win32_Render_Buffer* render_buffer, int x, int top, int bottom, uint32 color)
// {
//     uint8* pixel = ((uint8*)render_buffer->memory +
//             x * render_buffer->bytes_per_pixel +
//             top * render_buffer->pitch);
    
//     for (int Y = 0; Y < bottom; ++Y)
//     {
//         *(uint32*)pixel = color;
//         pixel += render_buffer->pitch;
//     }
// }
