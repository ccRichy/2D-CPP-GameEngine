#pragma once

//NOTE: WINDOWS/BITMAP Colors represented as: ARGB | 0xBB GG RR AA
//      OUR Colors represented as:            RGBA | 0xAA BB GG RR

//NOTE: this is per the windows little endian standard, maybe move this or rename?



struct Color
{
    uint8 r, g, b, a;
};

inline uint32
color_struct_to_uint32(Color color)
{
    return ( (color.a << 24 | color.r << 16) | (color.g << 8) | (color.b) );
}


inline uint8 color_channel_get_transparent(uint8 channel_prev, uint8 channel_new, float32 alpha)
{
    float32 alphasub = 1.0f - alpha;
    return (uint8)(((channel_new) * alpha) + (alphasub * (channel_prev)));
}

inline Color
color_get_transparent(Color color_prev, Color color_new)
{
    float32 alpha = (float32)color_new.a / 255;
    float32 oneminusalpha = 1 - alpha;
    return {
        (uint8)((color_new.r * alpha) + (oneminusalpha * color_prev.r)),
        (uint8)((color_new.g * alpha) + (oneminusalpha * color_prev.g)),
        (uint8)((color_new.b * alpha) + (oneminusalpha * color_prev.b))
    };
}

Color
color_mult_value_rgb(Color input, float32 value)
{
    return {
        (uint8)((float32)input.r * value),
        (uint8)((float32)input.g * value),
        (uint8)((float32)input.b * value)
    };
};

Color
color_mult_value_rgba(Color input, float32 value)
{
    return {
        (uint8)((float32)input.r * value),
        (uint8)((float32)input.g * value),
        (uint8)((float32)input.b * value),
        (uint8)((float32)input.a * value)
    };
};


#define LIGHTGRAY  Color{ 200, 200, 200, 255 }
#define GRAY       Color{ 130, 130, 130, 255 }
#define DARKGRAY   Color{ 80, 80, 80, 255 }   
#define YELLOW     Color{ 253, 249, 0, 255 }  
#define GOLD       Color{ 255, 203, 0, 255 }  
#define ORANGE     Color{ 255, 161, 0, 255 }  
#define PINK       Color{ 255, 109, 194, 255 }
#define RED        Color{ 230, 41, 55, 255 }  
#define MAROON     Color{ 190, 33, 55, 255 }  
#define GREEN      Color{ 0, 228, 48, 255 }   
#define LIME       Color{ 0, 158, 47, 255 }   
#define DARKGREEN  Color{ 0, 117, 44, 255 }   
#define SKYBLUE    Color{ 102, 191, 255, 255 }
#define BLUE       Color{ 0, 121, 241, 255 }  
#define DARKBLUE   Color{ 0, 82, 172, 255 }   
#define PURPLE     Color{ 200, 122, 255, 255 }
#define VIOLET     Color{ 135, 60, 190, 255 } 
#define DARKPURPLE Color{ 112, 31, 126, 255 } 
#define BEIGE      Color{ 211, 176, 131, 255 }
#define BROWN      Color{ 127, 106, 79, 255 } 
#define DARKBROWN  Color{ 76, 63, 47, 255 }   

#define WHITE      Color{ 255, 255, 255, 255 }
#define BLACK      Color{ 0, 0, 0, 255 }      
#define BLANK      Color{ 0, 0, 0, 0 }        
#define MAGENTA    Color{ 255, 0, 255, 255 }  
#define RAYWHITE   Color{ 245, 245, 245, 255 }
