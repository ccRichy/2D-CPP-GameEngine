/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Connor Ritchotte $
   ======================================================================== */

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

//TODO: 64-bit versions of functions where appropriate
#define in_range(value, min, max) (value >= min && value <= max)
#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))



//types
struct Vec2i
{
    union {
        struct { int32 x, y; };
        struct { int32 w, h; };
    };

    Vec2i& operator+=(const Vec2i& other) {
        x += other.x;
        y += other.y;
        return *this; 
    }
    Vec2i operator+(const Vec2i& other) const {
        Vec2i result = *this;
        result += other;
        return result;
    }
    
    Vec2i& operator-=(const Vec2i& other) {
        x -= other.x;
        y -= other.y;
        return *this; 
    }
    Vec2i operator-(const Vec2i& other) const {
        Vec2i result = *this;
        result -= other;
        return result;
    }

    Vec2i& operator*=(const Vec2i& other) {
        x *= other.x;
        y *= other.y;
        return *this; 
    }
    Vec2i operator*(const Vec2i& other) const {
        Vec2i result = *this;
        result *= other;
        return result;
    }
    Vec2i& operator/=(const Vec2i& other) {
        x /= other.x;
        y /= other.y;
        return *this; 
    }
    Vec2i operator/(const Vec2i& other) const {
        Vec2i result = *this;
        result /= other;
        return result;
    }
};
struct Vec2f
{
    union {
        struct { float32 x, y; };
        struct { float32 w, h; };
    };

    Vec2f& operator+=(const Vec2f& other) {
        x += other.x;
        y += other.y;
        return *this; 
    }
    Vec2f operator+(const Vec2f& other) const {
        Vec2f result = *this;
        result += other;
        return result;
    }
    
    Vec2f& operator-=(const Vec2f& other) {
        x -= other.x;
        y -= other.y;
        return *this; 
    }
    Vec2f operator-(const Vec2f& other) const {
        Vec2f result = *this;
        result -= other;
        return result;
    }

    Vec2f& operator*=(const Vec2f& other) {
        x *= other.x;
        y *= other.y;
        return *this; 
    }
    Vec2f operator*(const Vec2f& other) const {
        Vec2f result = *this;
        result *= other;
        return result;
    }
    Vec2f& operator/=(const Vec2f& other) {
        x /= other.x;
        y /= other.y;
        return *this; 
    }
    Vec2f operator/(const Vec2f& other) const {
        Vec2f result = *this;
        result /= other;
        return result;
    }
};

struct Rectangle
{
    Vec2f pos;
    Vec2f size;
};

//Alternate Names
typedef Vec2f V2f;
typedef Vec2f Vec2f;
typedef Vec2f Vector2f;
typedef Vec2i V2i;
typedef Vec2i Vec2i;
typedef Vec2i Vector2i;
typedef Rectangle Rect;




//funcs
inline uint32 //TODO: move to math
safe_truncate_uint64(uint64 value)
{
    Assert(value <= 0xFFFFFFFF);
    return (uint32)value;
}



inline int32
sign(int32 value)
{
    int32 result = 0;
    if (value > 0) result =  1;
    if (value < 0) result = -1;
    return result;
}
inline int32
sign(float32 value)
{
    int32 result = 0;
    if (value > 0) result =  1;
    if (value < 0) result = -1;
    return result;
}


inline int32
abs_i32(int32 value)
{
    int32 result = value;
    if (value < 0) result = value * -1;
    return result;
}
inline float32
abs_f32(float32 value)
{
    float32 result = value;
    if (value < 0) result = value * -1;
    return result;
}

inline int32
round_i32(float32 value)
{
    int32 result = (int32)roundf(value);
    return result;
}
inline float32
round_f32(float32 value)
{
    float32 result = roundf(value);
    return result;
}
// inline float64
// round_arbitrary(float64 input, float64 round_point)
// {
//     float64 result = input;
//     float64 remainder = fmod(input, round_point);
//     result -= remainder;

//     bool32 should_round_up = (remainder > round_point / 2);
//     if (should_round_up) result += round_point;

//     return result;
// }


inline int32
floor_i32(float32 value)
{
    int32 result = (int32)floorf(value);
    return result;
}
inline float32
floor_f32(float32 value)
{
    float32 result = floorf(value);
    return result;
}
inline int32
ceil_i32(float32 value)
{
    int32 result = (int32)floorf(value + 1);
    return result;
}
inline float32
ceil_f32(float32 value)
{
    float32 result = floorf(value + 1);
    return result;
}


inline float
clamp(float value, float min, float max)
{
    float result = (value < min ? min : value);
    if (result > max) result = max;
    return result;
}
inline float32
map_value(float32 value, Vec2f range_input, Vec2f range_output)
{
    float32 result = range_output.x + ((range_output.y - range_output.x) / (range_input.y - range_input.x)) * (value - range_input.x);
    return result;
}


inline float32
approach(float32 value, float32 dest, float32 spd)
{
    float32 result;
    if (value < dest)
        result = MIN(value + spd, dest); 
    else
        result = MAX(value - spd, dest);
    return result;
}

inline float32
lerp(float32 value, float32 dest, float32 spd)
{
    float32 result = value * (1.0f - spd) + (dest * spd);
    return result;
}



//vector functions
inline Vec2f round_v2f(Vec2f vector)
{
    Vec2f result = {
        round_f32(vector.x),
        round_f32(vector.y)
    };
    return result;
}
inline Vec2i round_v2i(Vec2f vector)
{
    Vec2i result = {
        round_i32(vector.x),
        round_i32(vector.y)
    };
    return result;
}

inline Vec2i
v2f_to_v2i(Vec2f vector)
{
    Vec2i result = {(i32)vector.x, (i32)vector.y};
    return result;
}

inline Vec2f
v2i_to_v2f(Vec2i vector)
{
    Vec2f result = {(f32)vector.x, (f32)vector.y};
    return result;
}
