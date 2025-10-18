#include "my_string.h"


#define LOOP_MAX 1024 //NOTE: increase this number as needed

//special
inline bool32
is_char_num(char character)
{
    bool32 result = (character >= '0' && character <= '9');
    return result;
}


void string_append(char* charbuff, char* string_to_append)
{
    int32 index_append_pos = string_length(charbuff); //asumes 0
    int32 loop_amt = string_length(string_to_append); //asumes 0
    for (int char_index = 0; char_index < loop_amt; ++char_index)
    {
        charbuff[char_index + index_append_pos] = string_to_append[char_index];
    }
}
void string_append(char* charbuff, const char* string_to_append)
{
    int32 index_append_pos = string_length(charbuff); //asumes 0
    int32 loop_amt = string_length(string_to_append); //asumes 0
    for (int char_index = 0; char_index < loop_amt; ++char_index)
    {
        charbuff[char_index + index_append_pos] = string_to_append[char_index];
    }    
}

//standard
void string_cat(char* buffer, char* string_pre, char* string_post)
{
    for (int i = 0; i < LOOP_MAX; ++i)
    {
        if (string_pre[i] != 0)
        {
            buffer[i] = string_pre[i];
        }
        else
        {
            for (int it = 0; it < LOOP_MAX; ++it)
            {
                buffer[i + it] = string_post[it];
                if (string_post[it] == 0)
                    break;
            }
            break;
        }
    }
}
void
string_cat(char* buffer, const char* string_pre, const char* string_post)
{
    for (int i = 0; i < LOOP_MAX; ++i)
    {
        if (string_pre[i] != 0)
        {
            buffer[i] = string_pre[i];
        }
        else
        {
            for (int it = 0; it < LOOP_MAX; ++it)
            {
                buffer[i + it] = string_post[it];
                if (string_post[it] == 0)
                    break;
            }
            break;
        }
    }
}



int32
string_length(char* string)
{
    int32 result = 0;
    for (int char_pos = 0; char_pos < 4096; ++char_pos)
    {
        if (string[char_pos] == 0) break;
        result++;
    }
    return result;
}
int32
string_length(const char* string)
{
    int32 result = 0;
    for (int char_pos = 0; char_pos < 4096; ++char_pos)
    {
        if (string[char_pos] == 0) break;
        result++;
    }
    return result;
}


bool32 string_contains(char* string, char character)
{
    b32 result = false;
    i32 in = 0;
    char ch;
    do {
        ch = string[in++];
        if (ch == character){
            result = true;
            break;
        }
    }
    while( ch != 0 );

    return result;
}

bool32
string_equals(char* string1, char* string2)
{
    bool32 result = true;
    for (int i = 0; i < 1024; ++i)
    {
        char char1 = string1[i];
        char char2 = string2[i];
        if (string1[i] != string2[i])
        {
            result = false;
            break;
        }
        else if (char1 == 0 || char2 == 0)
            break;
    }
    return result;
}
bool32
string_equals(const char* string1, const char* string2)
{
    bool32 result = true;
    for (int i = 0; i < 1024; ++i)
    {
        const char char1 = string1[i];
        const char char2 = string2[i];
        if (string1[i] != string2[i])
        {
            result = false;
            break;
        }
        else if (char1 == 0 || char2 == 0)
            break;
    }
    return result;
}
bool32 string_equals(char* string1, const char* string2)
{
    bool32 result = true;
    for (int i = 0; i < 1024; ++i)
    {
        const char char1 = string1[i];
        const char char2 = string2[i];
        if (string1[i] != string2[i])
        {
            result = false;
            break;
        }
        else if (char1 == 0 || char2 == 0)
            break;
    }
    return result;
    
}
bool32 string_equals(const char* string1, char* string2)
{
    bool32 result = true;
    for (int i = 0; i < 1024; ++i)
    {
        const char char1 = string1[i];
        const char char2 = string2[i];
        if (string1[i] != string2[i])
        {
            result = false;
            break;
        }
        else if (char1 == 0 || char2 == 0)
            break;
    }
    return result;
}




int32
string_get_until_delimiter(char* buffer, char* string, char delimiter)
{
    i32 ci = 0;
    do {
        buffer[ci] = string[ci];
    }
    while( string[++ci] != delimiter );
    buffer[ci] = 0;

    return ci;
}
int32 
string_get_until_space(char* buffer, char* string)
{
    i32 ci = 0;
    while( string[ci] != 0 ){
        if (string[ci] == ' ' || string[ci] == '\n') break;
        buffer[ci] = string[ci++];
    }
    buffer[ci] = 0;
    return ci; //string length
}



//TODO: automatically cast floats to ints?
int32
string_get_i32(const char* string)
{
    i32 result = 0;
    i32 exponent = 1;

    i32 ascii_num_offset = 48; //NOTE: maybe just use char '0'
    i32 length = string_length(string);
    for (i32 i = length; i >= 0; --i)
    {
        char character = string[i];
        if (character == 0 || !is_char_num(character)) continue; //HACK: 
        i32 digit = character - ascii_num_offset;
        result += digit * exponent;
        exponent *= 10;
    }
    
    return result;
}
//TODO: currently suffers from float imprecision
// float64 string_get_f32(const char* string)
// {
//     float64 result = 0;
//     float32 exponent = 1.0f;

//     int32 ascii_num_offset = 48; //NOTE: maybe just use char '0'
//     int32 length = string_length(string);
//     for (int i = length-1; i >= 0; --i)
//     {
//         char character = string[i];
//         if (character == '.')
//         {
//             result /= exponent;
//             exponent = 1;
//         }
//         else
//         {
//             int32 digit = character - ascii_num_offset;
//             result += digit * exponent;
//             exponent *= 10.0f;
//         }
//     }

//     return result;
//     // return (float32)result;
// }
