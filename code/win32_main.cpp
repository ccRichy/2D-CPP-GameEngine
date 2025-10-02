//
#include <math.h>

#include "my_types_constants.h"
#include "array_functions.h"
#include "my_math.cpp"
#include "my_string.cpp"
#include "render.h"
#include "game.h"

#include <stdio.h>
#include <windows.h>
#include <xinput.h>
#include <xaudio2.h>
#include "win32.cpp"
#include "win32_xaudio.cpp"


/*TODO:
  - Logging to file
  - Fix game dll double loading bug
*/


globalvar Win32_Render_Buffer Global_Render_Buffer;
globalvar bool32 Global_Running = true;

globalvar bool32 Global_BGMode_Enabled = false;
globalvar bool32 Global_BGMode_TransOutOfFocus = true;


globalvar bool32 dll_flip; //NOTE: hacky solution for game_dll auto hotloading
//BUG: something is preventing the game dll from loading in (majority of the time) without this
//Upon game dll compilation, we load it into the game by checking its create time vs our stored time.
//For some reason it still gets loaded twice (even with this hack), however-
//the 1 frame delay that this bool creates, HELPS prevent it from failing (for whatever reason)



//NOTE: KEYBOARD TPYING TEST
globalvar char global_typing_buffer[256];
globalvar int32 global_typing_index;



LRESULT CALLBACK
win32_main_window_callback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    LRESULT Result = 0;
    
    switch (message)
    {
        case WM_CHAR:{
            bool32 is_down = ((lparam & (1 << 31)) == 0);
            bool32 was_down = ((lparam & (1 << 30)) != 0);
            global_typing_buffer[global_typing_index] = (char)wparam;
            global_typing_index++;
        }break;
        
        case WM_ACTIVATEAPP:{
            if (!wparam) //out of focus
            {
                //BGMODE
                if (Global_BGMode_Enabled)
                {
                    if (Global_BGMode_TransOutOfFocus)
                        window_set_trans(window, true);
                }
                else
                {
                    window_set_trans(window, false);
                }

                OutputDebugStringA("(Win32)(WM_ACTIVATEAPP): lost focus\n");
            }
            else
            {
                //BGMODE
                if (Global_BGMode_Enabled)
                {
                    window_set_trans(window, false);
                }

                OutputDebugStringA("(Win32)(WM_ACTIVATEAPP): in focus\n");
                // else window_trans_enable(window, false);
            }
        }break;

        case WM_SIZE:{
            // OutputDebugStringA("(WM_SIZE)");
        }break;

        case WM_DESTROY:{
            Global_Running = false;
            OutputDebugStringA("(Win32)(WM_DESTROY)\n");
        }break;

        case WM_CLOSE:{
            Global_Running = false;
            OutputDebugStringA("(Win32)(WM_CLOSE)\n");
        }break;
        
        case WM_PAINT:{
            PAINTSTRUCT paint;
            HDC device_context = BeginPaint(window, &paint);
            //Win32_Client_Dimensions dimensions = win32_get_client_dimensions(window);
            win32_display_buffer_in_window(&Global_Render_Buffer, device_context, (int32)(BASE_W * Global_Settings->window_scale), (int32)(BASE_H * Global_Settings->window_scale));
            EndPaint(window, &paint);
            // OutputDebugStringA("(WM_PAINT)");
        }break;

        default:{
            Result = DefWindowProc(window, message, wparam, lparam);
        }break;
    }
    return (Result);
}




int CALLBACK
WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{    
    ShowCursor(FALSE);
    
    //performance query
    LARGE_INTEGER __perf_frequency_result;
    BOOL hardware_supports_highres_counter = QueryPerformanceFrequency(&__perf_frequency_result);
    global_cpu_freq = __perf_frequency_result.QuadPart;
    int64 tick_program_start = win32_get_tick();
    //SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);    
    UINT DesiredSchedulerMS = 1;
    bool32 sleep_is_granular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);    
    //granular my ass
    
    //NOTE: TEMP vars
    void* snd_buffer_test = VirtualAlloc(0, SND_BUFFER_SIZE_BYTES, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    
    //game code file paths
    char exe_path[300];
    win32_exe_path(exe_path, false);
    char dll_filename[] =       "game.dll";
    char dll_temp_filename[] =  "game_temp.dll";
    char dll_path[MAX_PATH];
    char dll_temp_path[MAX_PATH]; 
    string_cat(dll_path, exe_path, dll_filename);
    string_cat(dll_temp_path, exe_path, dll_temp_filename);
    
    //load Windows api
    win32_load_xinput();
    Win32_XAudio_Data xaudio2_data = win32_xaudio2_init();

    //game init
    
    //window
    int8 window_scale = WINDOW_SCALE_DEFAULT;
    
    win32_set_DIB(&Global_Render_Buffer, BASE_W * window_scale, BASE_H * window_scale);
    WNDCLASS window_class = {};
    window_class.style = CS_HREDRAW|CS_VREDRAW;
    window_class.lpfnWndProc = win32_main_window_callback;
    window_class.hInstance = instance;
    //WindowClass.hIcon;
    window_class.lpszClassName = "GameWindowClass";
    
    if (RegisterClass(&window_class))
    {
        OutputDebugStringA("(Win32): registered window class\n");

        RECT client_rect = {0, 0, BASE_W * window_scale, BASE_H * window_scale};
        DWORD window_style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE;
        AdjustWindowRectEx(&client_rect, window_style, FALSE, 0);
        HWND window = CreateWindowEx(
            WS_EX_LAYERED,
            window_class.lpszClassName,
            "game of year",
            window_style,
            //WS_OVERLAPPEDWINDOW|WS_VISIBLE,
            CW_USEDEFAULT, //x
            CW_USEDEFAULT, //y
            client_rect.right - client_rect.left, 
            client_rect.bottom - client_rect.top, 
            0,
            0,
            instance,
            0
        );

        
        if (window) //error
        {
            OutputDebugStringA("(Win32): window created successfully\n");
            
            BOOL win_set_attrib_result = SetLayeredWindowAttributes(window, 0, 255, LWA_ALPHA);
            // DWORD last_error = GetLastError();
            
#if MY_INTERNAL 
            LPVOID game_memory_address = (LPVOID)Terabytes(2);
#else
            LPVOID game_memory_address = 0;
#endif
                
            Game_Memory game_memory = {};
            game_memory.permanent_storage_space = Megabytes(GAME_MEMORY_MB_PERMANENT);
            game_memory.transient_storage_space = Gigabytes(GAME_MEMORY_MB_TRANSIENT);
            game_memory.DEBUG_platform_file_free_memory = DEBUG_platform_file_free_memory;
            game_memory.DEBUG_platform_file_read_entire = DEBUG_platform_file_read_entire;
            game_memory.DEBUG_platform_file_write_entire = DEBUG_platform_file_write_entire;
            
            uint64 game_memory_size_total = game_memory.permanent_storage_space + game_memory.transient_storage_space;
            game_memory.permanent_storage = VirtualAlloc(game_memory_address, game_memory_size_total, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

            game_memory.transient_storage = (uint8*)game_memory.permanent_storage + game_memory.permanent_storage_space;

            Game_Input_Map game_input_map = {};
            Game_Input_Map game_input_map_prev = {};

            
            if (game_memory.permanent_storage && game_memory.transient_storage)
            {
                Game_Render_Buffer game_render_buffer = {};
                Game_Sound_Buffer game_sound_buffer = {};
                Win32_Game_Code game_code = win32_load_game_code(dll_path, dll_temp_path);
                Game_Settings settings;
                Global_Settings = &settings; //NOTE: Global_Settings is for win32.cpp
                
                Win32_Sleep_Data sleep_data;
                sleep_data.estimate = 5e-3;
                sleep_data.mean = 5e-3;
                sleep_data.m2 = 0;
                sleep_data.count = 1;
                int64 tick_loop_start = win32_get_tick_diff(tick_program_start);
                float64 ms_loop_start = win32_tick_to_ms(tick_loop_start);
                                    
                Game_Pointers game_pointers = {};
                game_pointers.settings = &settings;
                game_pointers.memory   = &game_memory;
                game_pointers.render   = &game_render_buffer;
                game_pointers.sound    = &game_sound_buffer;
                //game_pointers.input //input gets passed by value
                
                while (Global_Running)
                {
                    //reload game code
                    FILETIME filetime = win32_file_get_write_time(dll_path);
                    if (0 != CompareFileTime(&filetime, &game_code.game_dll_last_write_time))
                    {
                        dll_flip = !dll_flip; //NOTE: HACK - see dll_flip
                        if (!dll_flip){
                            game_code.game_dll_last_write_time = filetime;
                            win32_unload_game_code(&game_code);
                            game_code = win32_load_game_code(dll_path, dll_temp_path);
                        }
                    }
                    else{
                        int cool = 1;
                    }

                    //TODO: make Game_Render_Buffer variables into pointers so we only set once?
                    game_render_buffer.memory = Global_Render_Buffer.memory;
                    game_render_buffer.width  = Global_Render_Buffer.width;
                    game_render_buffer.height = Global_Render_Buffer.height;
                    game_render_buffer.pitch  = Global_Render_Buffer.pitch;
                    
                    //input
                    game_input_map.mouse_scroll = 0;
                    win32_xinput_poll(&game_code, &game_input_map);
                    win32_process_pending_messages(&game_code, &game_input_map, &Global_Running);
                    win32_process_input(&game_input_map, &game_input_map_prev);
                    Game_Input_Map input = game_input_map;
                    
                    //BGMODE
                    if (input.debug_bgmode){
                        OutputDebugStringA("(Win32)(BGMode): ");
                        if (input.ctrl.hold){
                            if (Global_BGMode_Enabled){
                                Global_BGMode_TransOutOfFocus = !Global_BGMode_TransOutOfFocus;
                                auto toof_string = Global_BGMode_TransOutOfFocus ? "transparency out of focus enabled\n" : "transparency out of focus disabled\n";
                                OutputDebugStringA(toof_string);
                            }
                        }else{
                            Global_BGMode_Enabled = !Global_BGMode_Enabled;
                            window_set_topmost(window, Global_BGMode_Enabled);
                            window_set_trans(window, Global_BGMode_Enabled);
                            auto bgstatus_string = Global_BGMode_Enabled ? "Enabled\n" :  "Disabled\n";
                            OutputDebugStringA(bgstatus_string);
                        }
                    }
                    
                    if (input.debug_win_plus)
                        win32_set_window_scale(++Global_Settings->window_scale, window, &Global_Render_Buffer, &game_render_buffer);
                    
                    if (input.debug_win_minus)
                        win32_set_window_scale(--Global_Settings->window_scale, window, &Global_Render_Buffer, &game_render_buffer);


                    
                    //GAME LOOP
                    ZeroMemory(game_render_buffer.memory, Global_Render_Buffer.memory_size_bytes); //blacken buffer
                    if (game_code.update_and_draw)
                        game_code.update_and_draw(&game_pointers, game_input_map);

                    //render
                    HDC device_context = GetDC(window);
                    Win32_Client_Dimensions dimensions = win32_get_client_dimensions(window);
                    win32_display_buffer_in_window(&Global_Render_Buffer, device_context, dimensions.width, dimensions.height);
                    ReleaseDC(window, device_context);


                    
                    //sleep
                    int64 tick_spent_in_frame = win32_get_tick_diff(tick_loop_start);
                    float64 sec_spent_in_frame = win32_tick_to_sec(tick_spent_in_frame);

                    if (sec_spent_in_frame < SEC_PER_FRAME_TARGET)
                        sleep_well((SEC_PER_FRAME_TARGET) - sec_spent_in_frame, &sleep_data);


                    
                    //perf
                    int64 tick_loop_end = win32_get_tick();
                    float64 ms_this_frame = (float64)win32_tick_to_ms( tick_loop_end - tick_loop_start );
                    tick_loop_start = tick_loop_end;

                    //log //TODO: enable with preprocessor define?
                    char buffer[256];
                    sprintf_s(buffer, "ms/f: %.02f \n", ms_this_frame);
                    // OutputDebugStringA(buffer);
                }
            }
            else
            {
                OutputDebugStringA("(Win32): Game memory failed to allocate\n");
            }
        }
        else
        {
            OutputDebugStringA("(Win32): Window failed to be created\n");
        }
    }
    else
    {
        OutputDebugStringA("(Win32): Window class failed to register\n");
    }

    return 0;
}
