#pragma once

#include "windows.h"
#include "xinput.h"
#include "my_types_keywords.h"


struct Win32_Client_Dimensions
{
    int width;
    int height;
};

struct Win32_Render_Buffer
{    
    BITMAPINFO info;
    void* memory;
    int bytes_per_pixel;
    int pitch;
    int width;
    int height;
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
