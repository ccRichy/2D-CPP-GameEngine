#include "my_string.h"

void string_cat(char* buffer, char* string_pre, char* string_post)
{
    uint16 max_loops = 4096; //arbitary high number;
        
    for (int i = 0; i < max_loops; ++i)
    {
        if (string_pre[i] != 0)
        {
            buffer[i] = string_pre[i];
        }
        else
        {
            for (int it = 0; it < max_loops; ++it)
            {
                buffer[i + it] = string_post[it];
                if (string_post[it] == 0)
                    break;
            }
            break;
        }
    }
}
void string_cat(char* buffer, const char* string_pre, const char* string_post)
{
    uint16 max_loops = 4096; //arbitary high number;
        
    for (int i = 0; i < max_loops; ++i)
    {
        if (string_pre[i] != 0)
        {
            buffer[i] = string_pre[i];
        }
        else
        {
            for (int it = 0; it < max_loops; ++it)
            {
                buffer[i + it] = string_post[it];
                if (string_post[it] == 0)
                    break;
            }
            break;
        }
    }
}

int32 string_len(const char* string)
{
    int32 result = 0;
    for (int char_pos = 0; char_pos < 4096; ++char_pos)
    {
        if (string[char_pos] == 0) break;
        result++;
    }
    return result;
}

int32 string_len(char* string)
{
    int32 result = 0;
    for (int char_pos = 0; char_pos < 4096; ++char_pos)
    {
        if (string[char_pos] == 0) break;
        result++;
    }
    return result;
}
