#include "my_math.h"

int32 sign(int32 value)
{
    if (value > 0) return 1;
    if (value < 0) return -1;
    return 0;
}

int32 abs_i(int32 value)
{
    if (value < 0) return value * -1;
    return value;
}
float32 abs_f(float32 value)
{
    if (value < 0) return value * -1.0f;
    return value;
}

int32 floor(float32 value)
{
    return (int32)value;
}
int32 round(float32 value)
{
    return (int32)(value + 0.5);
}

float32 map_value(float32 value, Vec2 range_input, Vec2 range_output)
{
    return range_output.x + ((range_output.y - range_output.x) / (range_input.y - range_input.x)) * (value - range_input.x);
}
