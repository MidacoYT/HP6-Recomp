// The pointee types and member names for DAT_00fcaf34, DAT_00fcade0,
// and DAT_00cf1944 are unresolved; their observed offsets are retained.
LRESULT __stdcall FUN_005d8a30(
    HWND param_1, unsigned int param_2, unsigned int param_3, LPARAM param_4)
{
    PAINTSTRUCT paint;

    if (param_2 < 0x113) {
        if (param_2 == 0x112) {
            if (DAT_00866e2a != 0) {
                const unsigned int command = param_3 & 0xfff0;
                if (command < 0xf031) {
                    if (command == 0xf030 || command == 0xf000)
                        return 0;
                    if (command - 0xf010 == 0)
                        return 0;
                } else {
                    if (command == 0xf100)
                        return 0;
                    if (command - 0xf170 == 0)
                        return 0;
                }
                return -1;
            }
        } else {
            switch (param_2) {
            case 2:
                if (DAT_00866e2a != 0) {
                    do {
                    } while (ShowCursor(TRUE) < 1);
                }
                PostQuitMessage(0);
                return 0;

            case 5:
            case 0x14:
            case 0x1c:
                return 0;

            case 6:
                if (DAT_00866e2a != 0 && DAT_00fcaf34 != 0) {
                    if (static_cast<short>(param_3) == 0) {
                        FUN_005e0490();
                        FUN_006599b0();
                        do {
                        } while (ShowCursor(TRUE) < 1);

                        DAT_00cef68f = 1;
                        if (DAT_00cef55c == 0)
                            thunk_FUN_0100ea00();
                        DAT_00cef55c = DAT_00cef55c + 1;

                        if (DAT_00cef6a0 == 0) {
                            int* entry = *reinterpret_cast<int**>(
                                DAT_00fcade0 + 4);
                            DAT_00cef69c = 1;
                            if (entry != nullptr &&
                                (*entry == 0 ||
                                 *reinterpret_cast<int*>(*entry + 0xa0) == 0)) {
                                DAT_00cef69c = 0;
                                if (*entry != 0)
                                    FUN_00689110(reinterpret_cast<int*>(*entry));
                            }

                            DAT_00cef69d = 1;
                            const auto result = thunk_FUN_01007810();
                            if (static_cast<char>(result) == 0) {
                                DAT_00cef69d = static_cast<char>(result);
                                FUN_00545dc0('\0');
                            }
                        }

                        DAT_00cef6a0 = 0;
                        return 0;
                    }

                    if (*reinterpret_cast<int*>(DAT_00fcaf34 + 0xc) != 0) {
                        // Virtual method signatures remain unresolved.
                        auto* object = *reinterpret_cast<int**>(
                            DAT_00fcaf34 + 0xc);
                        auto* vtable = *reinterpret_cast<void***>(object);
                        using GetObject = int (__thiscall*)(void*);
                        const int value =
                            reinterpret_cast<GetObject>(vtable[4])(object);

                        auto* target = *reinterpret_cast<int**>(value + 0x218);
                        if (target != nullptr) {
                            auto* targetVtable =
                                *reinterpret_cast<void***>(target);
                            using Notify = void (__thiscall*)(void*);
                            reinterpret_cast<Notify>(targetVtable[7])(target);
                        }
                    }

                    FUN_00659920();
                    do {
                    } while (ShowCursor(FALSE) > 0);

                    if (DAT_00cef55c != 0) {
                        DAT_00cef55c = DAT_00cef55c - 1;
                        if (DAT_00cef55c == 0)
                            thunk_FUN_0100e9b0();
                    }
                    DAT_00cef68f = 0;
                    DAT_00cef6a0 = 2000;
                }
                return 0;

            case 7:
            case 0x20:
                if (DAT_00cef68f == 0) {
                    do {
                    } while (ShowCursor(FALSE) > 0);

                    if (DAT_00cf1944 != nullptr) {
                        auto* vtable =
                            *reinterpret_cast<void***>(DAT_00cf1944);
                        using SetState = void (__thiscall*)(void*, int);
                        reinterpret_cast<SetState>(vtable[12])(
                            DAT_00cf1944, 0);
                        return 0;
                    }
                } else {
                    HCURSOR cursor =
                        LoadCursorA(nullptr, MAKEINTRESOURCEA(0x7f00));
                    SetCursor(cursor);
                    FUN_005e0490();
                    FUN_006599b0();
                    do {
                    } while (ShowCursor(TRUE) < 1);

                    if (DAT_00cef55c == 0)
                        thunk_FUN_0100ea00();
                    DAT_00cef55c = DAT_00cef55c + 1;
                }
                return 0;

            case 0xf:
                if (DAT_00866e2a == 0) {
                    BeginPaint(param_1, &paint);
                    EndPaint(param_1, &paint);
                } else {
                    ValidateRect(param_1, nullptr);
                }
                return 0;

            case 0x10:
                if (DAT_00866e2a == 0)
                    FUN_005d82b0(param_1);
                DestroyWindow(param_1);
                return 0;

            case 0x84:
                if (DAT_00866e2a != 0)
                    return 1;
                break;
            }
        }
    } else if (param_2 == 0x120) {
        if (DAT_00866e2a != 0)
            return 0x10000;
    } else if (param_2 == 0x218) {
        if (param_3 != 0 && param_3 != 7)
            return -1;
        return 1;
    }

    return DefWindowProcA(param_1, param_2, param_3, param_4);
}