//
#include <math.h>
#include "array_functions.h"
#include "my_types_keywords.h"

#include "my_math.cpp"
#include "my_string.cpp"
#include "game.h"

#include <stdio.h>
#include <windows.h>
#include <xinput.h>
#include <xaudio2.h>
#include "win32.cpp"
#include "win32_xaudio.cpp"



//TODO: global for now
globalvar bool32 Global_Running = true;
globalvar Win32_Render_Buffer Global_Render_Buffer;
globalvar int64 global_perf_cpu_ticks_per_second;


//casdaisdnajiasin
LRESULT CALLBACK win32_main_window_callback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    LRESULT Result = 0;
    
    switch (message)
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








internal Win32_Init_Error
win32_test(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
    //temp vars
    int monitor_refresh = 60;
    #define game_refresh 30
    float32 target_seconds_per_frame = 1.0f / (float32)game_refresh;
    void* snd_buffer_test = VirtualAlloc(0, SND_BUFFER_SIZE_BYTES, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    
    //load game code
    char exe_path[300];
    win32_exe_path(exe_path, false);
    char dll_filename[] =       "game.dll";
    char dll_temp_filename[] =  "game_temp.dll";
    char dll_path[MAX_PATH];
    char dll_temp_path[MAX_PATH]; 
    string_cat(dll_path, exe_path, dll_filename);
    string_cat(dll_temp_path, exe_path, dll_temp_filename);
    Win32_Game_Code game_code = win32_load_game_code(dll_path, dll_temp_path);
    
    //load platform api
    win32_load_xinput();
    Win32_XAudio_Data xaudio2_data = win32_xaudio2_init();
    
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
    QueryPerformanceFrequency(&perf_frequency_result); //returns cpu-clock's ticks/second (hardware/OS-level constant)
    global_perf_cpu_ticks_per_second = perf_frequency_result.QuadPart;
    
    
    //register window
    if (RegisterClass(&window_class))
    {
        HWND window = CreateWindowEx(
            0,
            window_class.lpszClassName,
            "game of year",
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

        if (window) //error
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
                while (Global_Running)
                {
                    //reload game code
                    FILETIME filetime = win32_file_get_write_time(dll_path);
                    if (0 != CompareFileTime(&filetime, &game_code.game_dll_last_write_time))
                    {
                        game_code.game_dll_last_write_time = filetime;
                        win32_reload_game_code(game_code, dll_path, dll_temp_path);
                    }
                    
                    //input
                    win32_xinput_poll(&game_code, &game_input_map);
                    win32_process_pending_messages(&game_code, &game_input_map, &Global_Running);
                    
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
    win32_unload_game_code(&game_code);
    DeleteFile(dll_temp_path);
    return Win32_Init_Error::SUCCESS_NO_ERROR;
}







int CALLBACK
WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
    //temp vars
    int monitor_refresh = 60;
    #define game_refresh 30
    float32 target_seconds_per_frame = 1.0f / (float32)game_refresh;
    void* snd_buffer_test = VirtualAlloc(0, SND_BUFFER_SIZE_BYTES, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    
    //load game code
    char exe_path[300];
    win32_exe_path(exe_path, false);
    char dll_filename[] =       "game.dll";
    char dll_temp_filename[] =  "game_temp.dll";
    char dll_path[MAX_PATH];
    char dll_temp_path[MAX_PATH]; 
    string_cat(dll_path, exe_path, dll_filename);
    string_cat(dll_temp_path, exe_path, dll_temp_filename);
    Win32_Game_Code game_code = win32_load_game_code(dll_path, dll_temp_path);
    
    //load platform api
    win32_load_xinput();
    Win32_XAudio_Data xaudio2_data = win32_xaudio2_init();
    
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
    QueryPerformanceFrequency(&perf_frequency_result); //returns cpu-clock's ticks/second (hardware/OS-level constant)
    global_perf_cpu_ticks_per_second = perf_frequency_result.QuadPart;
    
    
    //register window
    if (RegisterClass(&window_class))
    {
        HWND window = CreateWindowEx(
            0,
            window_class.lpszClassName,
            "game of year",
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

        if (window) //error
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
                while (Global_Running)
                {
                    //reload game code
                    FILETIME filetime = win32_file_get_write_time(dll_path);
                    if (0 != CompareFileTime(&filetime, &game_code.game_dll_last_write_time))
                    {
                        game_code.game_dll_last_write_time = filetime;
                        win32_reload_game_code(game_code, dll_path, dll_temp_path);
                    }
                    
                    //input
                    win32_xinput_poll(&game_code, &game_input_map);
                    win32_process_pending_messages(&game_code, &game_input_map, &Global_Running);
                    
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
    win32_unload_game_code(&game_code);
    DeleteFile(dll_temp_path);
    return 0;
}
