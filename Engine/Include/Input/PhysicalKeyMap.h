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
            case KEY_A: out = KeyA;
                return true;
            case KEY_B: out = KeyB;
                return true;
            case KEY_C: out = KeyC;
                return true;
            case KEY_D: out = KeyD;
                return true;
            case KEY_E: out = KeyE;
                return true;
            case KEY_F: out = KeyF;
                return true;
            case KEY_G: out = KeyG;
                return true;
            case KEY_H: out = KeyH;
                return true;
            case KEY_I: out = KeyI;
                return true;
            case KEY_J: out = KeyJ;
                return true;
            case KEY_K: out = KeyK;
                return true;
            case KEY_L: out = KeyL;
                return true;
            case KEY_M: out = KeyM;
                return true;
            case KEY_N: out = KeyN;
                return true;
            case KEY_O: out = KeyO;
                return true;
            case KEY_P: out = KeyP;
                return true;
            case KEY_Q: out = KeyQ;
                return true;
            case KEY_R: out = KeyR;
                return true;
            case KEY_S: out = KeyS;
                return true;
            case KEY_T: out = KeyT;
                return true;
            case KEY_U: out = KeyU;
                return true;
            case KEY_V: out = KeyV;
                return true;
            case KEY_W: out = KeyW;
                return true;
            case KEY_X: out = KeyX;
                return true;
            case KEY_Y: out = KeyY;
                return true;
            case KEY_Z: out = KeyZ;
                return true;

            case KEY_1: out = Key1;
                return true;
            case KEY_2: out = Key2;
                return true;
            case KEY_3: out = Key3;
                return true;
            case KEY_4: out = Key4;
                return true;
            case KEY_5: out = Key5;
                return true;
            case KEY_6: out = Key6;
                return true;
            case KEY_7: out = Key7;
                return true;
            case KEY_8: out = Key8;
                return true;
            case KEY_9: out = Key9;
                return true;
            case KEY_0: out = Key0;
                return true;

            case KEY_F1: out = KeyF1;
                return true;
            case KEY_F2: out = KeyF2;
                return true;
            case KEY_F3: out = KeyF3;
                return true;
            case KEY_F4: out = KeyF4;
                return true;
            case KEY_F5: out = KeyF5;
                return true;
            case KEY_F6: out = KeyF6;
                return true;
            case KEY_F7: out = KeyF7;
                return true;
            case KEY_F8: out = KeyF8;
                return true;
            case KEY_F9: out = KeyF9;
                return true;
            case KEY_F10: out = KeyF10;
                return true;
            case KEY_F11: out = KeyF11;
                return true;
            case KEY_F12: out = KeyF12;
                return true;

            case KEY_ESC: out = KeyEscape;
                return true;
            case KEY_TAB: out = KeyTab;
                return true;
            case KEY_CAPSLOCK: out = KeyCapsLock;
                return true;
            case KEY_ENTER: out = KeyReturn;
                return true;
            case KEY_SPACE: out = KeySpace;
                return true;
            case KEY_BACKSPACE: out = KeyBackSpace;
                return true;
            case KEY_LEFTSHIFT: out = KeyShiftL;
                return true;
            case KEY_RIGHTSHIFT: out = KeyShiftR;
                return true;
            case KEY_LEFTCTRL: out = KeyCtrlL;
                return true;
            case KEY_RIGHTCTRL: out = KeyCtrlR;
                return true;
            case KEY_LEFTALT: out = KeyAltL;
                return true;
            case KEY_RIGHTALT: out = KeyAltR;
                return true;
            case KEY_LEFTMETA: out = KeySuperL;
                return true;
            case KEY_RIGHTMETA: out = KeySuperR;
                return true;
            case KEY_COMPOSE: out = KeyMenu;
                return true;

            case KEY_MINUS: out = KeyMinus;
                return true;
            case KEY_EQUAL: out = KeyEqual;
                return true;
            case KEY_LEFTBRACE: out = KeyBracketLeft;
                return true;
            case KEY_RIGHTBRACE: out = KeyBracketRight;
                return true;
            case KEY_SEMICOLON: out = KeySemicolon;
                return true;
            case KEY_APOSTROPHE: out = KeyApostrophe;
                return true;
            case KEY_GRAVE: out = KeyGrave;
                return true;
            case KEY_BACKSLASH: out = KeyBackslash;
                return true;
            case KEY_COMMA: out = KeyComma;
                return true;
            case KEY_DOT: out = KeyPeriod;
                return true;
            case KEY_SLASH: out = KeySlash;
                return true;
            case KEY_102ND: out = KeyLess;
                return true;

            case KEY_UP: out = KeyUp;
                return true;
            case KEY_DOWN: out = KeyDown;
                return true;
            case KEY_LEFT: out = KeyLeft;
                return true;
            case KEY_RIGHT: out = KeyRight;
                return true;
            case KEY_HOME: out = KeyHome;
                return true;
            case KEY_END: out = KeyEnd;
                return true;
            case KEY_PAGEUP: out = KeyPageUp;
                return true;
            case KEY_PAGEDOWN: out = KeyPageDown;
                return true;
            case KEY_INSERT: out = KeyInsert;
                return true;
            case KEY_DELETE: out = KeyDelete;
                return true;

            case KEY_SYSRQ: out = KeyPrint;
                return true;
            case KEY_SCROLLLOCK: out = KeyScrollLock;
                return true;
            case KEY_PAUSE: out = KeyBreak;
                return true;
            case KEY_NUMLOCK: out = KeyNumLock;
                return true;

            case KEY_KP0: out = KeyKpInsert;
                return true;
            case KEY_KP1: out = KeyKpEnd;
                return true;
            case KEY_KP2: out = KeyKpDown;
                return true;
            case KEY_KP3: out = KeyKpPageDown;
                return true;
            case KEY_KP4: out = KeyKpLeft;
                return true;
            case KEY_KP5: out = KeyKpBegin;
                return true;
            case KEY_KP6: out = KeyKpRight;
                return true;
            case KEY_KP7: out = KeyKpHome;
                return true;
            case KEY_KP8: out = KeyKpUp;
                return true;
            case KEY_KP9: out = KeyKpPageUp;
                return true;
            case KEY_KPDOT: out = KeyKpDelete;
                return true;
            case KEY_KPENTER: out = KeyKpEnter;
                return true;
            case KEY_KPPLUS: out = KeyKpAdd;
                return true;
            case KEY_KPMINUS: out = KeyKpSubtract;
                return true;
            case KEY_KPASTERISK: out = KeyKpMultiply;
                return true;
            case KEY_KPSLASH: out = KeyKpDivide;
                return true;
            case KEY_KPEQUAL: out = KeyKpEqual;
                return true;
        }

        return false;
    }

#endif

#if defined(_WIN32)
    inline bool PhysicalToGainput(unsigned code, bool ext, gainput::Key &out) {
        using namespace gainput;

        switch (code) {
            case 0x1E: out = KeyA;
                return true;
            case 0x30: out = KeyB;
                return true;
            case 0x2E: out = KeyC;
                return true;
            case 0x20: out = KeyD;
                return true;
            case 0x12: out = KeyE;
                return true;
            case 0x21: out = KeyF;
                return true;
            case 0x22: out = KeyG;
                return true;
            case 0x23: out = KeyH;
                return true;
            case 0x17: out = KeyI;
                return true;
            case 0x24: out = KeyJ;
                return true;
            case 0x25: out = KeyK;
                return true;
            case 0x26: out = KeyL;
                return true;
            case 0x32: out = KeyM;
                return true;
            case 0x31: out = KeyN;
                return true;
            case 0x18: out = KeyO;
                return true;
            case 0x19: out = KeyP;
                return true;
            case 0x10: out = KeyQ;
                return true;
            case 0x13: out = KeyR;
                return true;
            case 0x1F: out = KeyS;
                return true;
            case 0x14: out = KeyT;
                return true;
            case 0x16: out = KeyU;
                return true;
            case 0x2F: out = KeyV;
                return true;
            case 0x11: out = KeyW;
                return true;
            case 0x2D: out = KeyX;
                return true;
            case 0x15: out = KeyY;
                return true;
            case 0x2C: out = KeyZ;
                return true;

            case 0x02: out = Key1;
                return true;
            case 0x03: out = Key2;
                return true;
            case 0x04: out = Key3;
                return true;
            case 0x05: out = Key4;
                return true;
            case 0x06: out = Key5;
                return true;
            case 0x07: out = Key6;
                return true;
            case 0x08: out = Key7;
                return true;
            case 0x09: out = Key8;
                return true;
            case 0x0A: out = Key9;
                return true;
            case 0x0B: out = Key0;
                return true;

            case 0x3B: out = KeyF1;
                return true;
            case 0x3C: out = KeyF2;
                return true;
            case 0x3D: out = KeyF3;
                return true;
            case 0x3E: out = KeyF4;
                return true;
            case 0x3F: out = KeyF5;
                return true;
            case 0x40: out = KeyF6;
                return true;
            case 0x41: out = KeyF7;
                return true;
            case 0x42: out = KeyF8;
                return true;
            case 0x43: out = KeyF9;
                return true;
            case 0x44: out = KeyF10;
                return true;
            case 0x57: out = KeyF11;
                return true;
            case 0x58: out = KeyF12;
                return true;

            case 0x01: out = KeyEscape;
                return true;
            case 0x0F: out = KeyTab;
                return true;
            case 0x3A: out = KeyCapsLock;
                return true;
            case 0x0E: out = KeyBackSpace;
                return true;
            case 0x39: out = KeySpace;
                return true;
            case 0x2A: out = KeyShiftL;
                return true;
            case 0x36: out = KeyShiftR;
                return true;
            case 0x1D: out = ext ? KeyCtrlR : KeyCtrlL;
                return true;
            case 0x38: out = ext ? KeyAltR : KeyAltL;
                return true;
            case 0x1C: out = ext ? KeyKpEnter : KeyReturn;
                return true;
            case 0x5B: if (ext) {
                    out = KeySuperL;
                    return true;
                }
                return false;
            case 0x5C: if (ext) {
                    out = KeySuperR;
                    return true;
                }
                return false;
            case 0x5D: if (ext) {
                    out = KeyMenu;
                    return true;
                }
                return false;

            case 0x0C: out = KeyMinus;
                return true;
            case 0x0D: out = KeyEqual;
                return true;
            case 0x1A: out = KeyBracketLeft;
                return true;
            case 0x1B: out = KeyBracketRight;
                return true;
            case 0x27: out = KeySemicolon;
                return true;
            case 0x28: out = KeyApostrophe;
                return true;
            case 0x29: out = KeyGrave;
                return true;
            case 0x2B: out = KeyBackslash;
                return true;
            case 0x33: out = KeyComma;
                return true;
            case 0x34: out = KeyPeriod;
                return true;
            case 0x35: out = ext ? KeyKpDivide : KeySlash;
                return true;
            case 0x56: out = KeyLess;
                return true;

            case 0x37: out = ext ? KeyPrint : KeyKpMultiply;
                return true;
            case 0x46: out = KeyScrollLock;
                return true;
            case 0x45: out = KeyNumLock;
                return true;

            case 0x47: out = ext ? KeyHome : KeyKpHome;
                return true;
            case 0x48: out = ext ? KeyUp : KeyKpUp;
                return true;
            case 0x49: out = ext ? KeyPageUp : KeyKpPageUp;
                return true;
            case 0x4B: out = ext ? KeyLeft : KeyKpLeft;
                return true;
            case 0x4C: out = KeyKpBegin;
                return true;
            case 0x4D: out = ext ? KeyRight : KeyKpRight;
                return true;
            case 0x4F: out = ext ? KeyEnd : KeyKpEnd;
                return true;
            case 0x50: out = ext ? KeyDown : KeyKpDown;
                return true;
            case 0x51: out = ext ? KeyPageDown : KeyKpPageDown;
                return true;
            case 0x52: out = ext ? KeyInsert : KeyKpInsert;
                return true;
            case 0x53: out = ext ? KeyDelete : KeyKpDelete;
                return true;
            case 0x4A: out = KeyKpSubtract;
                return true;
            case 0x4E: out = KeyKpAdd;
                return true;
        }
        return false;
    }

#endif
}
