//
#include <math.h>
#include "array_functions.h"
#include "my_types_keywords.h"

#include "my_math.cpp"
#include "game.h"

#include <stdio.h>
#include <windows.h>
#include <xinput.h>
#include <xaudio2.h>
#include "win32.cpp"



#define STREAMING_BUFFER_SIZE 65536
#define MAX_BUFFER_COUNT 3
BYTE buffers[MAX_BUFFER_COUNT][STREAMING_BUFFER_SIZE];


#include "win32_xaudio.cpp"


//TODO: global for now
globalvar bool32 Global_Running = true;
globalvar Win32_Render_Buffer Global_Render_Buffer;
globalvar int64 global_perf_cpu_ticks_per_second;



LRESULT CALLBACK win32_main_window_callback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    LRESULT Result = 0;
    
    switch(message)
    {
        case WM_SIZE:{
        }break;

        case WM_DESTROY:{
            Global_Running = false;
            OutputDebugStringA("WM_DESTROY\n");
        }break;

        case WM_CLOSE:{
            Global_Running = false;
            OutputDebugStringA("WM_CLOSE\n");
        }break;

        case WM_ACTIVATEAPP:{
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        }break;
        
        case WM_PAINT:{
            PAINTSTRUCT paint;
            HDC device_context = BeginPaint(window, &paint);
            Win32_Client_Dimensions dimensions = win32_get_client_dimensions(window);
            win32_display_buffer_in_window(&Global_Render_Buffer, device_context, dimensions.width, dimensions.height);
            EndPaint(window, &paint);
        }break;

        default:{
            Result = DefWindowProc(window, message, wparam, lparam);
        }break;
    }

    return (Result);
}


internal void
win32_process_pending_messages(Win32_Game_Code* game_code, Game_Input_Map* game_input_map)
{
    MSG message;
    while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
    {
        switch(message.message)
        {
            case WM_QUIT:{
                Global_Running = false;
            }break;
                            
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:{
                //TODO: more robust swapping between input devices
                if (game_input_device != Game_Input_Device::keyboard_mouse)
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
                        Global_Running = false;
                    if (vk_code == VK_ESCAPE)
                        Global_Running = false;
                }
            }break;

            default:{
                TranslateMessage(&message);
                DispatchMessageA(&message);                                
            }break;
        }
                    
    }

}


int CALLBACK
WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
    //init data
    win32_load_xinput();

    int monitor_refresh = 60;
    #define game_refresh 30
    float32 target_seconds_per_frame = 1.0f / (float32)game_refresh;

    //window
    win32_resize_DIB_section(&Global_Render_Buffer, 1280, 720);
    WNDCLASS window_class = {};
    window_class.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    window_class.lpfnWndProc = win32_main_window_callback;
    window_class.hInstance = instance;
    //WindowClass.hIcon;
    window_class.lpszClassName = "GameWindowClass";

    //performance
    LARGE_INTEGER perf_frequency_result;
    QueryPerformanceFrequency(&perf_frequency_result); //returns overall cpu ticks/second (hardware/OS-level constant)
    global_perf_cpu_ticks_per_second = perf_frequency_result.QuadPart;


    //SOUND BULLSHIT
    Win32_XAudio_Data xaudio2_data = win32_xaudio2_init();
    int64 currentBufferIndex = 0;
    void* snd_buffer_test = VirtualAlloc(0, SND_BUFFER_SIZE_BYTES, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    
    //register window
    if (RegisterClass(&window_class))
    {
        HWND window = CreateWindowEx(
            0,
            window_class.lpszClassName,
            "HMH",
            WS_OVERLAPPEDWINDOW|WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            instance,
            0
        );

        if (window)
        {
            HDC device_context = GetDC(window);

            //
#if MY_INTERNAL 
            LPVOID game_memory_address = (LPVOID)Terabytes(2);
#else
            LPVOID game_memory_address = 0;
#endif
                
            Game_Memory game_memory = {};
            game_memory.permanent_storage_space = Megabytes(64);
            game_memory.transient_storage_space = Gigabytes(1);
            
            uint64 game_memory_size_total = game_memory.permanent_storage_space + game_memory.transient_storage_space;
            game_memory.permanent_storage = VirtualAlloc(game_memory_address, game_memory_size_total, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

            game_memory.transient_storage = (uint8*)game_memory.permanent_storage + game_memory.permanent_storage_space;

            Game_Input_Map game_input_map = {};
            Game_Input_Map game_input_map_prev = {};

            
            if (game_memory.permanent_storage && game_memory.transient_storage)
            {

                const char* dll_filename = "../build/game.dll";
                Win32_Game_Code game_code = win32_load_game_code();
                game_code.game_dll_last_write_time = win32_file_get_write_time(dll_filename);
                
                while(Global_Running)
                {
                    //reload game code
                    FILETIME filetime = win32_file_get_write_time(dll_filename);
                    if (0 != CompareFileTime(&filetime, &game_code.game_dll_last_write_time))
                    {
                        win32_reload_game_code(game_code);
                    }
                    
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
                            if ((game_input_device != Game_Input_Device::controller) &&
                                (pad->wButtons ||
                                 abs_i(pad->sThumbLX) > deadzone_l*3 || abs_i(pad->sThumbLY) > deadzone_l*3 ||
                                 abs_i(pad->sThumbRX) > deadzone_l*3 || abs_i(pad->sThumbLY) > deadzone_l*3 ||
                                 l_trig > 100 || r_trig > 100))
                            {
                                game_code.input_change_device(Game_Input_Device::controller, &game_input_map);
                            }

                            //send out inputs
                            //TODO: abstration layer for rebinding
                            if (game_input_device == Game_Input_Device::controller)
                            {
                                Game_Input_Map* _in = &game_input_map;

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
                        }
                    }

                    //KEYBOARD
                    win32_process_pending_messages(&game_code, &game_input_map);
                    
                    Game_Offscreen_Buffer render_buffer;
                    render_buffer.memory = Global_Render_Buffer.memory;
                    render_buffer.width  = Global_Render_Buffer.width;
                    render_buffer.height = Global_Render_Buffer.height;
                    render_buffer.pitch  = Global_Render_Buffer.pitch;

                    //game loop
                    if (game_input_map.up.press)
                       DEBUG_xaudio2_play_sound("select1.wav", &xaudio2_data, snd_buffer_test);
                    if (game_input_map.down.press)
                       DEBUG_xaudio2_play_sound("male_hurt_04.wav", &xaudio2_data, snd_buffer_test);
                    if (game_input_map.left.press)
                        win32_reload_game_code(game_code);                        

                    
                    win32_process_input(&game_input_map, &game_input_map_prev);
                    game_code.update_and_draw(&game_memory, game_input_map, &render_buffer);


                    Win32_Client_Dimensions dimensions = win32_get_client_dimensions(window);
                    win32_display_buffer_in_window(&Global_Render_Buffer, device_context, dimensions.width, dimensions.height);

#if 0
                    char buffer[256];
                    float32 kilocycles_per_frame = (float32)perf_cycles_this_frame / 1000.0f;
                    sprintf_s(buffer, "fps: %.02f | ms/f: %.02f | kc/f: %.02f\n", 0.0f, perf_ms_per_frame,  kilocycles_per_frame);
                    OutputDebugStringA(buffer);
#endif
                }
            }
            else
            {
                //TODO: Log - memory failed to allocated
            }
        }
        else
        {
            //TODO: log - window failed to be created
        }
    }
    else
    {
        //TODO: log - window class failed to register
    }

    //delete trash
    DeleteFile("game_temp.dll");
    
    return 0;
}
