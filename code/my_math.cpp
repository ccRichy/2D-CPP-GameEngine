#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)


//TODO: 64-bit versions of functions where appropriate

#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))


struct Vec2
{
    float32 x;
    float32 y;

    Vec2& operator+=(const Vec2& other) {
        x += other.x;
        y += other.y;
        return *this; // return the modified object
    }
    Vec2 operator+(const Vec2& other) const {
        Vec2 result = *this;
        result += other;
        return result;
    }
    
    Vec2& operator-=(const Vec2& other) {
        x -= other.x;
        y -= other.y;
        return *this; // return the modified object
    }
    Vec2 operator-(const Vec2& other) const {
        Vec2 result = *this;
        result -= other;
        return result;
    }
};



inline int32
sign(int32 value)
{
    if (value > 0) return 1;
    if (value < 0) return -1;
    return 0;
}
inline int32
sign(float32 value)
{
    if (value > 0) return 1;
    if (value < 0) return -1;
    return 0;
}


inline int32
abs_i32(int32 value)
{
    if (value < 0) return value * -1;
    return value;
}
inline float32
abs_f32(float32 value)
{
    if (value < 0) return value * -1.0f;
    return value;
}


inline int32
round_i32(float32 value)
{
    return (int32)(value + 0.5);
}
inline float32
round_f32(float32 value)
{
    return (float32)(int32)(value + 0.5);
}


inline float
clamp(float value, float min, float max)
{
    float result = (value < min)? min : value;

    if (result > max) result = max;

    return result;
}
inline float32
map_value(float32 value, Vec2 range_input, Vec2 range_output)
{
    return range_output.x + ((range_output.y - range_output.x) / (range_input.y - range_input.x)) * (value - range_input.x);
}


inline float32
approach(float32 value, float32 dest, float32 spd)
{
      if (value < dest)
          return MIN(value + spd, dest); 
      else
          return MAX(value - spd, dest);
}
