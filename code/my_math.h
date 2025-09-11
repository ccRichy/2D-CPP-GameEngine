#pragma once
#include "my_vec.h"

//TODO: 64-bit versions
#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))



int32 sign(int32 value);

int32   abs_i(int32 value);
float32 abs_f(float32 value);

int32 floor(float32 value);
int32 round(float32 value);

float32 clamp(float value, float min, float max);
float32 map_value(float32 value, Vec2 range_input, Vec2 range_output);


float approach(float value, float dest, float spd);
