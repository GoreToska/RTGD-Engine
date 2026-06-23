//
// Created by ivan on 6/23/26.
//

#pragma once
#include <gainput/gainput.h>

#if defined(__linux__)
#include <linux/input-event-codes.h>
#endif

#if defined(_WIN32)

#endif

namespace RTGDEngine {
#if defined(__linux__)
    inline bool PhysicalToGainput(int code, gainput::Key &out) {
        using namespace gainput;

        switch (code) {
            case KEY_W: out = KeyW;
                return true;
            case KEY_A: out = KeyA;
                return true;
            case KEY_S: out = KeyS;
                return true;
            case KEY_D: out = KeyD;
                return true;
            case KEY_Q: out = KeyQ;
                return true;
            case KEY_E: out = KeyE;
                return true;
            case KEY_LEFTSHIFT: out = KeyShiftL;
                return true;
            case KEY_RIGHTSHIFT: out = KeyShiftR;
                return true;
            case KEY_LEFTCTRL: out = KeyCtrlL;
                return true;
            case KEY_LEFTALT: out = KeyAltL;
                return true;
            case KEY_SPACE: out = KeySpace;
                return true;
            case KEY_ESC: out = KeyEscape;
                return true;
            case KEY_ENTER: out = KeyReturn;
                return true;
            case KEY_TAB: out = KeyTab;
                return true;
            case KEY_UP: out = KeyUp;
                return true;
            case KEY_DOWN: out = KeyDown;
                return true;
            case KEY_LEFT: out = KeyLeft;
                return true;
            case KEY_RIGHT: out = KeyRight;
                return true;
        }

        return false;
    }

#endif

#if defined(_WIN32)
    inline bool PhysicalToGainput(unsigned code, bool ext, gainput::Key &out) {
        using namespace gainput;

        switch (code) {
            case 0x11: out = KeyW;
                return true;
            case 0x1E: out = KeyA;
                return true;
            case 0x1F: out = KeyS;
                return true;
            case 0x20: out = KeyD;
                return true;
            case 0x10: out = KeyQ;
                return true;
            case 0x12: out = KeyE;
                return true;
            case 0x2A: out = KeyShiftL;
                return true;
            case 0x36: out = KeyShiftR;
                return true;
            case 0x1D: out = ext ? KeyCtrlR : KeyCtrlL;
                return true;

            // extended = right
            case 0x38: out = ext ? KeyAltR : KeyAltL;
                return true;
            case 0x39: out = KeySpace;
                return true;
            case 0x01: out = KeyEscape;
                return true;
            case 0x1C: out = ext ? KeyKpEnter : KeyReturn;
                return true;
            case 0x48: if (ext) {
                    out = KeyUp;
                    return true;
                }
                return false;

            case 0x50: if (ext) {
                    out = KeyDown;
                    return true;
                }
                return false;
            case 0x4B: if (ext) {
                    out = KeyLeft;
                    return true;
                }
                return false;
            case 0x4D: if (ext) {
                    out = KeyRight;
                    return true;
                }
        }
        return false;
    }

#endif
}
