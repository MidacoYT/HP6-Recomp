#include <cstdarg>
#include <cstdio>

void __cdecl FUN_0063ccf0(char* param_1, ...)
{
    char local_800[2048];

    va_list args;
    va_start(args, param_1);
    std::vsprintf(local_800, param_1, args);
    va_end(args);
}