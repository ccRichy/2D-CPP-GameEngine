#pragma once

#define array_length(array) (sizeof(array) / sizeof(array[0]))

#define array_shift_left(array, length, nullvalue)\
for (int __it = 0; __it < length; ++__it)         \
{                                                 \
    if (__it+1 < length)                          \
        array[__it] = array[__it+1];              \
    else                                          \
        array[__it] = nullvalue;                  \
}                                                 \

#define array_erase_index(array, length, index, nullvalue)\
for (int __it = index; __it < length; ++__it)             \
{                                                         \
    if (__it+1 < length)                                  \
        array[__it] = array[__it+1];                      \
    else                                                  \
        array[__it] = nullvalue;                          \
}                                                         \
