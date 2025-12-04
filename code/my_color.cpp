/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */
// #include "my_color.h"



inline uint8 color_channel_get_transparent(uint8 channel_prev, uint8 channel_new, float32 alpha)
{
    float32 alphasub = 1.0f - alpha;
    return (uint8)(((channel_new) * alpha) + (alphasub * (channel_prev)));
}

inline Color //TODO: optimize
color_get_transparent(Color color_prev, Color color_new)
{
    float32 alpha = (float32)color_new.a / 255;
    float32 oneminusalpha = 1 - alpha;
    return {
        (uint8)((color_new.b * alpha) + (oneminusalpha * color_prev.b)),
        (uint8)((color_new.g * alpha) + (oneminusalpha * color_prev.g)),
        (uint8)((color_new.r * alpha) + (oneminusalpha * color_prev.r)),
        255
    };
}

// Color
// color_sub_rgb(Color input, u8 sub)
// {
//     return {
//         input.r - sub,
//         input.g - sub,
//         input.b - sub
//     };
// };

Color
color_mult_rgb(Color input, float32 mult)
{
    return {
        (uint8)((float32)input.r * mult),
        (uint8)((float32)input.g * mult),
        (uint8)((float32)input.b * mult)
    };
};

Color
color_mult_rgba(Color input, float32 mult)
{
    return {
        (uint8)((float32)input.r * mult),
        (uint8)((float32)input.g * mult),
        (uint8)((float32)input.b * mult),
        (uint8)((float32)input.a * mult)
    };
};
