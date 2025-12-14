/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */
// #pragma once



//NOTE: this is per the windows little endian standard (BGRA/BGRX), maybe move this or rename?
struct Color
{
    union {
        struct { uint8 b, g, r, a; };
        struct { uint8 blue, green, red, alpha; };
        uint32 decimal;
    };
};


//NOTE: BGRA
#define LIGHTGRAY  Color{ 200, 200, 200, 255 }
#define GRAY       Color{ 130, 130, 130, 255 }
#define DARKGRAY   Color{ 80, 80, 80, 255 }   
#define YELLOW     Color{ 0, 249, 253, 255 }  
#define GOLD       Color{ 0, 203, 255, 255 }  
#define ORANGE     Color{ 0, 161, 255, 255 }  
#define PINK       Color{ 194, 109, 255, 255 }
#define RED        Color{ 55, 41, 230, 255 }  
#define MAROON     Color{ 55, 33, 190, 255 }
#define GREEN      Color{ 48, 228, 0, 255 }   
#define LIME       Color{ 47, 158, 0, 255 }   
#define DARKGREEN  Color{ 44, 117, 0, 255 }   
#define SKYBLUE    Color{ 255, 191, 102, 255 }
#define BLUE       Color{ 241, 121, 0, 255 }  
#define DARKBLUE   Color{ 172, 82, 0, 255 }   
#define PURPLE     Color{ 255, 122, 200, 255 }
#define VIOLET     Color{ 190, 60, 135, 255 } 
#define DARKPURPLE Color{ 126, 31, 112, 255 } 
#define BEIGE      Color{ 131, 176, 211, 255 }
#define BROWN      Color{ 79, 106, 127, 255 } 
#define DARKBROWN  Color{ 47, 63, 76, 255 }   

#define WHITE      Color{ 255, 255, 255, 255 }
#define BLACK      Color{ 0, 0, 0, 255 }      
#define BLANK      Color{ 0, 0, 0, 0 }        
#define MAGENTA    Color{ 255, 0, 255, 255 }  
#define RAYWHITE   Color{ 245, 245, 245, 255 }

//debug colors
#define BBOXRED   Color{ 55, 41, 230, 150 }



uint8 color_channel_get_transparent(uint8 channel_prev, uint8 channel_new, float32 alpha);
Color color_get_transparent(Color color_prev, Color color_new);
Color color_mult_value_rgb(Color input, float32 value);
Color color_mult_value_rgba(Color input, float32 value);




// struct Color
// {
//     uint8 r, g, b, a;
// };

// inline uint32
// color_struct_to_uint32(Color color)
// {
//     return ( (color.a << 24 | color.r << 16) | (color.g << 8) | (color.b) );
// }


////NOTE: rgba alignment
// #define LIGHTGRAY  Color{ 200, 200, 200, 255 }
// #define GRAY       Color{ 130, 130, 130, 255 }
// #define DARKGRAY   Color{ 80, 80, 80, 255 }   
// #define YELLOW     Color{ 253, 249, 0, 255 }  
// #define GOLD       Color{ 255, 203, 0, 255 }  
// #define ORANGE     Color{ 255, 161, 0, 255 }  
// #define PINK       Color{ 255, 109, 194, 255 }
// #define RED        Color{ 230, 41, 55, 255 }  
// #define MAROON     Color{ 190, 33, 55, 255 }  
// #define GREEN      Color{ 0, 228, 48, 255 }   
// #define LIME       Color{ 0, 158, 47, 255 }   
// #define DARKGREEN  Color{ 0, 117, 44, 255 }   
// #define SKYBLUE    Color{ 102, 191, 255, 255 }
// #define BLUE       Color{ 0, 121, 241, 255 }  
// #define DARKBLUE   Color{ 0, 82, 172, 255 }   
// #define PURPLE     Color{ 200, 122, 255, 255 }
// #define VIOLET     Color{ 135, 60, 190, 255 } 
// #define DARKPURPLE Color{ 112, 31, 126, 255 } 
// #define BEIGE      Color{ 211, 176, 131, 255 }
// #define BROWN      Color{ 127, 106, 79, 255 } 
// #define DARKBROWN  Color{ 76, 63, 47, 255 }   

// #define WHITE      Color{ 255, 255, 255, 255 }
// #define BLACK      Color{ 0, 0, 0, 255 }      
// #define BLANK      Color{ 0, 0, 0, 0 }        
// #define MAGENTA    Color{ 255, 0, 255, 255 }  
// #define RAYWHITE   Color{ 245, 245, 245, 255 }
