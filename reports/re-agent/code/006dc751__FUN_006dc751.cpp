unsigned char* __cdecl FUN_006dc751(
    unsigned char* param_1,
    int param_2,
    int param_3,
    unsigned int* param_4,
    unsigned int* param_5)
{
    const unsigned int trailerSize = FUN_006dc6a3(param_1, nullptr, nullptr);
    unsigned char* const end = param_1 + (FUN_006dc664(param_1) - trailerSize);

    int fieldWidth = 4;
    int valueWidth = 4;
    unsigned char* cursor = param_1 + 0x10;

    const int version = FUN_006dc5f9(param_1);
    if (version == 1) {
        fieldWidth = 3;
        valueWidth = 3;
        cursor = param_1 + 6;
    } else if (version == 3) {
        fieldWidth = static_cast<signed char>(param_1[3]) - '0';
        valueWidth = fieldWidth;
        if (valueWidth > 4) {
            valueWidth = 4;
        }
    }

    int index = 0;
    if (param_2 == 0) {
        while (cursor < end) {
            unsigned char* result = cursor + fieldWidth + valueWidth;
            if (index == param_3) {
                if (param_4 != nullptr) {
                    *param_4 = static_cast<unsigned int>(
                        FUN_006dc594(reinterpret_cast<int>(cursor), fieldWidth));
                }
                if (param_5 != nullptr) {
                    *param_5 = FUN_006dc532(cursor + fieldWidth, valueWidth);
                }
                return result;
            }

            while (*result++ != 0) {}
            ++index;
            cursor = result;
        }
    } else {
        while (cursor < end) {
            unsigned char* result = cursor + fieldWidth + valueWidth;
            if (FUN_006dc5c9(reinterpret_cast<char*>(result)) == 0) {
                if (param_4 != nullptr) {
                    *param_4 = static_cast<unsigned int>(
                        FUN_006dc594(reinterpret_cast<int>(cursor), fieldWidth));
                }
                if (param_5 != nullptr) {
                    *param_5 = FUN_006dc532(cursor + fieldWidth, valueWidth);
                }
                return result;
            }

            while (*result++ != 0) {}
            cursor = result;
        }
    }

    if (param_4 != nullptr) {
        *param_4 = 0;
    }
    if (param_5 != nullptr) {
        *param_5 = 0;
    }
    return nullptr;
}