#include "my_string.h"

void string_cat(char* buffer, char* string_pre, char* string_post)
{
    uint16 max_loops = 65535; //arbitary high number;
        
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
    uint16 max_loops = 65535; //arbitary high number;
        
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
