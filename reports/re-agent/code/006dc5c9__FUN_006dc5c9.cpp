// The binary receives param_1 in ECX and in_EAX in EAX.
// A compiler-specific entry wrapper is required to preserve that register ABI.
int FUN_006dc5c9_impl(char* param_1, char* in_EAX)
{
    for (;;) {
        const int first = tolower(static_cast<int>(*param_1));
        const int second = tolower(static_cast<int>(*in_EAX));

        if (first != second || *param_1 == '\0') {
            return first - second;
        }

        ++in_EAX;
        ++param_1;
    }
}