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
#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))

#define DO(expr) ({ expr; }) //TEST:

#define repeat(__times) for (int __it = 0; __it < __times; ++__it)
// #define repeat(times, expr) for (int __it = 0; __it < N; ++__it){ ({ expr; }) }
#define in_range(value, min, max) (value >= min && value <= max)
#define dist(value1, value2) (abs_f32(value1 - value2))



//types
//NOTE: declare as a struct member to "inherit" the names and keep under one name
//Q: why not hard-code the names? A: so we can see at a glance the names we made in this struct, and for expressiveness :)
#define Vec2fUnion(__vec_name, __xname, __yname)\
    union { struct {Vec2f __vec_name;}; struct {float32 __xname, __yname; }; }

#define Vec2iUnion(__vec_name, __xname, __yname)\
    union { struct {Vec2f __vec_name;}; struct {float32 __xname, __yname; }; }

struct Vector2i
{
    union {
        struct { int32 x, y; };
        struct { int32 w, h; };
    };

    Vector2i& operator+=(const Vector2i& other) {
        x += other.x;
        y += other.y;
        return *this; 
    }
    Vector2i operator+(const Vector2i& other) const {
        Vector2i result = *this;
        result += other;
        return result;
    }
    
    Vector2i& operator-=(const Vector2i& other) {
        x -= other.x;
        y -= other.y;
        return *this; 
    }
    Vector2i operator-(const Vector2i& other) const {
        Vector2i result = *this;
        result -= other;
        return result;
    }

    Vector2i& operator*=(const Vector2i& other) {
        x *= other.x;
        y *= other.y;
        return *this; 
    }
    Vector2i operator*(const Vector2i& other) const {
        Vector2i result = *this;
        result *= other;
        return result;
    }
    Vector2i& operator/=(const Vector2i& other) {
        x /= other.x;
        y /= other.y;
        return *this; 
    }
    Vector2i operator/(const Vector2i& other) const {
        Vector2i result = *this;
        result /= other;
        return result;
    }
};
struct Vector2f
{
    union {
        struct { float32 x, y; };
        struct { float32 w, h; };
    };

    Vector2f& operator+=(const Vector2f& other) {
        x += other.x;
        y += other.y;
        return *this; 
    }
    Vector2f operator+(const Vector2f& other) const {
        Vector2f result = *this;
        result += other;
        return result;
    }
    
    Vector2f& operator-=(const Vector2f& other) {
        x -= other.x;
        y -= other.y;
        return *this; 
    }
    Vector2f operator-(const Vector2f& other) const {
        Vector2f result = *this;
        result -= other;
        return result;
    }

    Vector2f& operator*=(const Vector2f& other) {
        x *= other.x;
        y *= other.y;
        return *this; 
    }
    Vector2f operator*(const Vector2f& other) const {
        Vector2f result = *this;
        result *= other;
        return result;
    }
    Vector2f& operator/=(const Vector2f& other) {
        x /= other.x;
        y /= other.y;
        return *this; 
    }
    Vector2f operator/(const Vector2f& other) const {
        Vector2f result = *this;
        result /= other;
        return result;
    }
};
typedef Vector2f V2f;
typedef Vector2f Vec2f;
typedef Vector2i V2i;
typedef Vector2i Vec2i;



struct Rectangle 
{
    union {
        struct { Vec2f pos, size; };
        struct { float32 x, y, w, h; };
    };
};

//Alternate Names
typedef Rectangle Rect;





//funcs
inline uint32 //TODO: move to math
safe_truncate_uint64(uint64 value)
{
    Assert(value <= 0xFFFFFFFF);
    return (uint32)value;
}



//intrinsics
// #pragma intrinsic(abs, fabs, floor, ceil, fmod, sqrt)


inline int32
abs_i32(int32 value)
{
    i32 result = abs(value);
    return result;
}

inline float32
abs_f32(float32 value)
{
    f32 result = abs(value);
    return result;
}


inline float32
mod_f32(f32 dividend, f32 divisor)
{
    f32 result = fmod(dividend, divisor);
    return result;
}


inline int32
floor_i32(float32 value)
{
    i32 result = (i32)floor(value);
    return result;
}
inline float32
floor_f32(float32 value)
{
    float32 result = floor(value);
    return result;
}

inline int32
ceil_i32(float32 value)
{
    int32 result = (int32)ceil(value);
    return result;
}
inline float32
ceil_f32(float32 value)
{
    float32 result = ceil(value);
    return result;
}

inline int32
round_i32(float32 value)
{
    // i32 result = _mm_cvtt_ss2si(_mm_set_ss(value));
    i32 result = (i32)round(value);
    return result;
}
inline float32
round_f32(float32 value)
{
    // f32 result = (f32)_mm_cvtt_ss2si(_mm_set_ss(value));
    f32 result = round(value);
    return result;
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


inline float32 sqrt_f32(float32 value)
{
    f32 result = sqrt(value);
    return result;
}
inline float64 sqrt_f64(float64 value)
{
    f64 result = sqrt(value);
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



inline float
clamp_f32(float value, float min, float max)
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
inline Vec2f
abs_v2f(Vec2f vector)
{
    Vec2f result = {
        abs_f32(vector.x),
        abs_f32(vector.y)
    };
    return result;
}
inline Vec2i
abs_v2i(Vec2i vector)
{
    Vec2i result = {
        abs_i32(vector.x),
        abs_i32(vector.y)
    };
    return result;
}
inline Vec2f
round_v2f(Vec2f vector)
{
    Vec2f result = {
        round_f32(vector.x),
        round_f32(vector.y)
    };
    return result;
}
inline Vec2i
round_v2i(Vec2f vector)
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
