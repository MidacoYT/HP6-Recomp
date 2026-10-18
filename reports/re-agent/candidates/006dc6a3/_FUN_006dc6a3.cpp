unsigned int __cdecl FUN_006dc6a3(
    unsigned char* param_1,
    int* param_2,
    unsigned int* param_3)
{
    const unsigned int size = FUN_006dc664(param_1);
    unsigned char* const tail = param_1 + (size - 8);

    const char first = static_cast<char>(tail[0]);
    if (!((first >= 'A' && first <= 'Z') ||
          (first >= 'a' && first <= 'z'))) {
        return 0;
    }

    const char second = static_cast<char>(tail[1]);
    if (second < '0' || second > '9') {
        return 0;
    }

    const char third = static_cast<char>(tail[2]);
    if (third < '0' || third > '9') {
        return 0;
    }

    const char fourth = static_cast<char>(tail[3]);
    if (fourth < '0' || fourth > '9') {
        return 0;
    }

    if (param_2 != nullptr) {
        *param_2 = fourth + -0x14d0 + (third + second * 10) * 10;
    }

    if (param_3 != nullptr) {
        *param_3 = FUN_006dc532(tail + 4, 4);
    }

    return 8;
}
