// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/kbd_conversion.h"
#include <xkbcommon/xkbcommon.h>

#include "base/logging.h"
#include "base/strings/stringprintf.h"

namespace ui {

KeyboardCode KeyboardCodeFromXKeysym(unsigned int keysym) {
  switch (keysym) {
    case XKB_KEY_BackSpace:
      return VKEY_BACK;
    case XKB_KEY_Delete:
    case XKB_KEY_KP_Delete:
      return VKEY_DELETE;
    case XKB_KEY_Tab:
    case XKB_KEY_KP_Tab:
    case XKB_KEY_ISO_Left_Tab:
      return VKEY_TAB;
    case XKB_KEY_Linefeed:
    case XKB_KEY_Return:
    case XKB_KEY_KP_Enter:
    case XKB_KEY_ISO_Enter:
      return VKEY_RETURN;
    case XKB_KEY_Clear:
    case XKB_KEY_KP_Begin:  // NumPad 5 without Num Lock, for crosbug.com/29169.
      return VKEY_CLEAR;
    case XKB_KEY_KP_Space:
    case XKB_KEY_space:
      return VKEY_SPACE;
    case XKB_KEY_Home:
    case XKB_KEY_KP_Home:
      return VKEY_HOME;
    case XKB_KEY_End:
    case XKB_KEY_KP_End:
      return VKEY_END;
    case XKB_KEY_Page_Up:
    case XKB_KEY_KP_Page_Up:  // aka XKB_KEY_KP_Prior
      return VKEY_PRIOR;
    case XKB_KEY_Page_Down:
    case XKB_KEY_KP_Page_Down:  // aka XKB_KEY_KP_Next
      return VKEY_NEXT;
    case XKB_KEY_Left:
    case XKB_KEY_KP_Left:
      return VKEY_LEFT;
    case XKB_KEY_Right:
    case XKB_KEY_KP_Right:
      return VKEY_RIGHT;
    case XKB_KEY_Down:
    case XKB_KEY_KP_Down:
      return VKEY_DOWN;
    case XKB_KEY_Up:
    case XKB_KEY_KP_Up:
      return VKEY_UP;
    case XKB_KEY_Escape:
      return VKEY_ESCAPE;
    case XKB_KEY_Kana_Lock:
    case XKB_KEY_Kana_Shift:
      return VKEY_KANA;
    case XKB_KEY_Hangul:
      return VKEY_HANGUL;
    case XKB_KEY_Hangul_Hanja:
      return VKEY_HANJA;
    case XKB_KEY_Kanji:
      return VKEY_KANJI;
    case XKB_KEY_Henkan:
      return VKEY_CONVERT;
    case XKB_KEY_Muhenkan:
      return VKEY_NONCONVERT;
    case XKB_KEY_Zenkaku_Hankaku:
      return VKEY_DBE_DBCSCHAR;
    case XKB_KEY_A:
    case XKB_KEY_a:
      return VKEY_A;
    case XKB_KEY_B:
    case XKB_KEY_b:
      return VKEY_B;
    case XKB_KEY_C:
    case XKB_KEY_c:
      return VKEY_C;
    case XKB_KEY_D:
    case XKB_KEY_d:
      return VKEY_D;
    case XKB_KEY_E:
    case XKB_KEY_e:
      return VKEY_E;
    case XKB_KEY_F:
    case XKB_KEY_f:
      return VKEY_F;
    case XKB_KEY_G:
    case XKB_KEY_g:
      return VKEY_G;
    case XKB_KEY_H:
    case XKB_KEY_h:
      return VKEY_H;
    case XKB_KEY_I:
    case XKB_KEY_i:
      return VKEY_I;
    case XKB_KEY_J:
    case XKB_KEY_j:
      return VKEY_J;
    case XKB_KEY_K:
    case XKB_KEY_k:
      return VKEY_K;
    case XKB_KEY_L:
    case XKB_KEY_l:
      return VKEY_L;
    case XKB_KEY_M:
    case XKB_KEY_m:
      return VKEY_M;
    case XKB_KEY_N:
    case XKB_KEY_n:
      return VKEY_N;
    case XKB_KEY_O:
    case XKB_KEY_o:
      return VKEY_O;
    case XKB_KEY_P:
    case XKB_KEY_p:
      return VKEY_P;
    case XKB_KEY_Q:
    case XKB_KEY_q:
      return VKEY_Q;
    case XKB_KEY_R:
    case XKB_KEY_r:
      return VKEY_R;
    case XKB_KEY_S:
    case XKB_KEY_s:
      return VKEY_S;
    case XKB_KEY_T:
    case XKB_KEY_t:
      return VKEY_T;
    case XKB_KEY_U:
    case XKB_KEY_u:
      return VKEY_U;
    case XKB_KEY_V:
    case XKB_KEY_v:
      return VKEY_V;
    case XKB_KEY_W:
    case XKB_KEY_w:
      return VKEY_W;
    case XKB_KEY_X:
    case XKB_KEY_x:
      return VKEY_X;
    case XKB_KEY_Y:
    case XKB_KEY_y:
      return VKEY_Y;
    case XKB_KEY_Z:
    case XKB_KEY_z:
      return VKEY_Z;

    case XKB_KEY_0:
    case XKB_KEY_1:
    case XKB_KEY_2:
    case XKB_KEY_3:
    case XKB_KEY_4:
    case XKB_KEY_5:
    case XKB_KEY_6:
    case XKB_KEY_7:
    case XKB_KEY_8:
    case XKB_KEY_9:
      return static_cast<KeyboardCode>(VKEY_0 + (keysym - XKB_KEY_0));

    case XKB_KEY_parenright:
      return VKEY_0;
    case XKB_KEY_exclam:
      return VKEY_1;
    case XKB_KEY_at:
      return VKEY_2;
    case XKB_KEY_numbersign:
      return VKEY_3;
    case XKB_KEY_dollar:
      return VKEY_4;
    case XKB_KEY_percent:
      return VKEY_5;
    case XKB_KEY_asciicircum:
      return VKEY_6;
    case XKB_KEY_ampersand:
      return VKEY_7;
    case XKB_KEY_asterisk:
      return VKEY_8;
    case XKB_KEY_parenleft:
      return VKEY_9;

    case XKB_KEY_KP_0:
    case XKB_KEY_KP_1:
    case XKB_KEY_KP_2:
    case XKB_KEY_KP_3:
    case XKB_KEY_KP_4:
    case XKB_KEY_KP_5:
    case XKB_KEY_KP_6:
    case XKB_KEY_KP_7:
    case XKB_KEY_KP_8:
    case XKB_KEY_KP_9:
      return static_cast<KeyboardCode>(VKEY_NUMPAD0 + (keysym - XKB_KEY_KP_0));

    case XKB_KEY_multiply:
    case XKB_KEY_KP_Multiply:
      return VKEY_MULTIPLY;
    case XKB_KEY_KP_Add:
      return VKEY_ADD;
    case XKB_KEY_KP_Separator:
      return VKEY_SEPARATOR;
    case XKB_KEY_KP_Subtract:
      return VKEY_SUBTRACT;
    case XKB_KEY_KP_Decimal:
      return VKEY_DECIMAL;
    case XKB_KEY_KP_Divide:
      return VKEY_DIVIDE;
    case XKB_KEY_KP_Equal:
    case XKB_KEY_equal:
    case XKB_KEY_plus:
      return VKEY_OEM_PLUS;
    case XKB_KEY_comma:
    case XKB_KEY_less:
      return VKEY_OEM_COMMA;
    case XKB_KEY_minus:
    case XKB_KEY_underscore:
      return VKEY_OEM_MINUS;
    case XKB_KEY_greater:
    case XKB_KEY_period:
      return VKEY_OEM_PERIOD;
    case XKB_KEY_colon:
    case XKB_KEY_semicolon:
      return VKEY_OEM_1;
    case XKB_KEY_question:
    case XKB_KEY_slash:
      return VKEY_OEM_2;
    case XKB_KEY_asciitilde:
    case XKB_KEY_quoteleft:
      return VKEY_OEM_3;
    case XKB_KEY_bracketleft:
    case XKB_KEY_braceleft:
      return VKEY_OEM_4;
    case XKB_KEY_backslash:
    case XKB_KEY_bar:
      return VKEY_OEM_5;
    case XKB_KEY_bracketright:
    case XKB_KEY_braceright:
      return VKEY_OEM_6;
    case XKB_KEY_quoteright:
    case XKB_KEY_quotedbl:
      return VKEY_OEM_7;
    case XKB_KEY_Shift_L:
    case XKB_KEY_Shift_R:
      return VKEY_SHIFT;
    case XKB_KEY_Control_L:
    case XKB_KEY_Control_R:
      return VKEY_CONTROL;
    case XKB_KEY_Meta_L:
    case XKB_KEY_Meta_R:
    case XKB_KEY_Alt_L:
    case XKB_KEY_Alt_R:
      return VKEY_MENU;
    case XKB_KEY_Pause:
      return VKEY_PAUSE;
    case XKB_KEY_Caps_Lock:
      return VKEY_CAPITAL;
    case XKB_KEY_Num_Lock:
      return VKEY_NUMLOCK;
    case XKB_KEY_Scroll_Lock:
      return VKEY_SCROLL;
    case XKB_KEY_Select:
      return VKEY_SELECT;
    case XKB_KEY_Print:
      return VKEY_PRINT;
    case XKB_KEY_Execute:
      return VKEY_EXECUTE;
    case XKB_KEY_Insert:
    case XKB_KEY_KP_Insert:
      return VKEY_INSERT;
    case XKB_KEY_Help:
      return VKEY_HELP;
    case XKB_KEY_Super_L:
      return VKEY_LWIN;
    case XKB_KEY_Super_R:
      return VKEY_RWIN;
    case XKB_KEY_Menu:
      return VKEY_APPS;
    case XKB_KEY_F1:
    case XKB_KEY_F2:
    case XKB_KEY_F3:
    case XKB_KEY_F4:
    case XKB_KEY_F5:
    case XKB_KEY_F6:
    case XKB_KEY_F7:
    case XKB_KEY_F8:
    case XKB_KEY_F9:
    case XKB_KEY_F10:
    case XKB_KEY_F11:
    case XKB_KEY_F12:
    case XKB_KEY_F13:
    case XKB_KEY_F14:
    case XKB_KEY_F15:
    case XKB_KEY_F16:
    case XKB_KEY_F17:
    case XKB_KEY_F18:
    case XKB_KEY_F19:
    case XKB_KEY_F20:
    case XKB_KEY_F21:
    case XKB_KEY_F22:
    case XKB_KEY_F23:
    case XKB_KEY_F24:
      return static_cast<KeyboardCode>(VKEY_F1 + (keysym - XKB_KEY_F1));
    case XKB_KEY_KP_F1:
    case XKB_KEY_KP_F2:
    case XKB_KEY_KP_F3:
    case XKB_KEY_KP_F4:
      return static_cast<KeyboardCode>(VKEY_F1 + (keysym - XKB_KEY_KP_F1));

    case XKB_KEY_guillemotleft:
    case XKB_KEY_guillemotright:
    case XKB_KEY_degree:
    // In the case of canadian multilingual keyboard layout, VKEY_OEM_102 is
    // assigned to ugrave key.
    case XKB_KEY_ugrave:
    case XKB_KEY_Ugrave:
      return VKEY_OEM_102;  // international backslash key in 102 keyboard.
  }
  DLOG(WARNING) << "Unknown keysym: " << base::StringPrintf("0x%x", keysym);
  return VKEY_UNKNOWN;
}

}  // namespace ui

