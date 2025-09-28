#include "my_math.h"

int32 sign(int32 value)
{
    if (value > 0) return 1;
    if (value < 0) return -1;
    return 0;
}
int32 sign(float32 value)
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


int32 round_i32(float32 value)
{
    return (int32)(value + 0.5);
}
float32 round_f32(float32 value)
{
    return (float32)(int32)(value + 0.5);
}


float clamp(float value, float min, float max)
{
    float result = (value < min)? min : value;

    if (result > max) result = max;

    return result;
}
float32 map_value(float32 value, Vec2 range_input, Vec2 range_output)
{
    return range_output.x + ((range_output.y - range_output.x) / (range_input.y - range_input.x)) * (value - range_input.x);
}


float approach(float32 value, float32 dest, float32 spd)
{
      if (value < dest)
          return MIN(value + spd, dest); 
      else
          return MAX(value - spd, dest);
}
