#pragma once

#define TO_STRING(x) #x

void string_cat(char* charbuff, char* string_pre, char* string_post);
void string_cat(char* charbuff, const char* string_pre, const char* string_post);

int32 string_length(char* string);
int32 string_length(const char* string);

bool32 string_equals(char* string1, char* string2);
bool32 string_equals(const char* string1, const char* string2);
