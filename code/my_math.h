#pragma once
#include "my_vec.h"

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

//TODO: 64-bit versions
#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))


int32 sign(int32 value);

int32   abs_i(int32 value);
float32 abs_f(float32 value);

int32   round_i32(float32 value);
float32 round_f32(float32 value);

float32 clamp(float value, float min, float max);
float32 map_value(float32 value, Vec2 range_input, Vec2 range_output);


float approach(float value, float dest, float spd);
