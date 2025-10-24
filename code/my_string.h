//NOTE: //REQUIRED: these functions assume null termination strings
#pragma once




#define To_String(x) #x

// #define String_Clear(string)  \
// int __iteration = 0;          \
// while (string[__iteration]){   \
//     string[__iteration] = 0;  \
//     ++count;                  \     
// }                             



enum struct Char_Type
{
    Number,
    Math,
    Letter,
    Alphanumeric,
    Printable,
    Whitespace,
    Visible
};


inline bool32 is_char_num(char c);
inline bool32 is_char_type(char c, Char_Type type);

void string_append(char* charbuff, char* string_to_append);
void string_append(char* charbuff, const char* string_to_append);

void string_cat(char* charbuff, char* string_pre, char* string_post);
void string_cat(char* charbuff, const char* string_pre, const char* string_post);

int32 string_length(char* string);
int32 string_length(const char* string);

bool32 string_contains_char(char* string, char character);
bool32 string_contains_string(char* string1, char* string2);

bool32 string_equals(char* string1, char* string2);
bool32 string_equals(const char* string1, const char* string2);
bool32 string_equals(char* string1, const char* string2);
bool32 string_equals(const char* string1, char* string2);

//return string length
int32 string_get_until_delimiter(char* buffer, char* string_start, char delimiter);
int32 string_get_until_space(char* buffer, char* string_start);

void string_clear(char* string);

int32 string_get_i32(const char* string);
// float32 string_get_f32(const char* string);
