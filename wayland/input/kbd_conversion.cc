// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/input/kbd_conversion.h"

#include <xkbcommon/xkbcommon.h>

#include "base/logging.h"
#include "base/strings/stringprintf.h"

namespace ozonewayland {

ui::KeyboardCode KeyboardCodeFromXKeysym(unsigned int keysym) {
  switch (keysym) {
    case XKB_KEY_BackSpace:
      return ui::VKEY_BACK;
    case XKB_KEY_Delete:
    case XKB_KEY_KP_Delete:
      return ui::VKEY_DELETE;
    case XKB_KEY_Tab:
    case XKB_KEY_KP_Tab:
    case XKB_KEY_ISO_Left_Tab:
      return ui::VKEY_TAB;
    case XKB_KEY_Linefeed:
    case XKB_KEY_Return:
    case XKB_KEY_KP_Enter:
    case XKB_KEY_ISO_Enter:
      return ui::VKEY_RETURN;
    case XKB_KEY_Clear:
    case XKB_KEY_KP_Begin:  // NumPad 5 without Num Lock, for crosbug.com/29169.
      return ui::VKEY_CLEAR;
    case XKB_KEY_KP_Space:
    case XKB_KEY_space:
      return ui::VKEY_SPACE;
    case XKB_KEY_Home:
    case XKB_KEY_KP_Home:
      return ui::VKEY_HOME;
    case XKB_KEY_End:
    case XKB_KEY_KP_End:
      return ui::VKEY_END;
    case XKB_KEY_Page_Up:
    case XKB_KEY_KP_Page_Up:  // aka XKB_KEY_KP_Prior
      return ui::VKEY_PRIOR;
    case XKB_KEY_Page_Down:
    case XKB_KEY_KP_Page_Down:  // aka XKB_KEY_KP_Next
      return ui::VKEY_NEXT;
    case XKB_KEY_Left:
    case XKB_KEY_KP_Left:
      return ui::VKEY_LEFT;
    case XKB_KEY_Right:
    case XKB_KEY_KP_Right:
      return ui::VKEY_RIGHT;
    case XKB_KEY_Down:
    case XKB_KEY_KP_Down:
      return ui::VKEY_DOWN;
    case XKB_KEY_Up:
    case XKB_KEY_KP_Up:
      return ui::VKEY_UP;
    case XKB_KEY_Escape:
      return ui::VKEY_ESCAPE;
    case XKB_KEY_Kana_Lock:
    case XKB_KEY_Kana_Shift:
      return ui::VKEY_KANA;
    case XKB_KEY_Hangul:
      return ui::VKEY_HANGUL;
    case XKB_KEY_Hangul_Hanja:
      return ui::VKEY_HANJA;
    case XKB_KEY_Kanji:
      return ui::VKEY_KANJI;
    case XKB_KEY_Henkan:
      return ui::VKEY_CONVERT;
    case XKB_KEY_Muhenkan:
      return ui::VKEY_NONCONVERT;
    case XKB_KEY_Zenkaku_Hankaku:
      return ui::VKEY_DBE_DBCSCHAR;
    case XKB_KEY_A:
    case XKB_KEY_a:
      return ui::VKEY_A;
    case XKB_KEY_B:
    case XKB_KEY_b:
      return ui::VKEY_B;
    case XKB_KEY_C:
    case XKB_KEY_c:
      return ui::VKEY_C;
    case XKB_KEY_D:
    case XKB_KEY_d:
      return ui::VKEY_D;
    case XKB_KEY_E:
    case XKB_KEY_e:
      return ui::VKEY_E;
    case XKB_KEY_F:
    case XKB_KEY_f:
      return ui::VKEY_F;
    case XKB_KEY_G:
    case XKB_KEY_g:
      return ui:: VKEY_G;
    case XKB_KEY_H:
    case XKB_KEY_h:
      return ui::VKEY_H;
    case XKB_KEY_I:
    case XKB_KEY_i:
      return ui::VKEY_I;
    case XKB_KEY_J:
    case XKB_KEY_j:
      return ui::VKEY_J;
    case XKB_KEY_K:
    case XKB_KEY_k:
      return ui::VKEY_K;
    case XKB_KEY_L:
    case XKB_KEY_l:
      return ui::VKEY_L;
    case XKB_KEY_M:
    case XKB_KEY_m:
      return ui::VKEY_M;
    case XKB_KEY_N:
    case XKB_KEY_n:
      return ui::VKEY_N;
    case XKB_KEY_O:
    case XKB_KEY_o:
      return ui::VKEY_O;
    case XKB_KEY_P:
    case XKB_KEY_p:
      return ui::VKEY_P;
    case XKB_KEY_Q:
    case XKB_KEY_q:
      return ui::VKEY_Q;
    case XKB_KEY_R:
    case XKB_KEY_r:
      return ui::VKEY_R;
    case XKB_KEY_S:
    case XKB_KEY_s:
      return ui::VKEY_S;
    case XKB_KEY_T:
    case XKB_KEY_t:
      return ui::VKEY_T;
    case XKB_KEY_U:
    case XKB_KEY_u:
      return ui::VKEY_U;
    case XKB_KEY_V:
    case XKB_KEY_v:
      return ui::VKEY_V;
    case XKB_KEY_W:
    case XKB_KEY_w:
      return ui::VKEY_W;
    case XKB_KEY_X:
    case XKB_KEY_x:
      return ui::VKEY_X;
    case XKB_KEY_Y:
    case XKB_KEY_y:
      return ui::VKEY_Y;
    case XKB_KEY_Z:
    case XKB_KEY_z:
      return ui::VKEY_Z;

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
      return static_cast<ui::KeyboardCode>(ui::VKEY_0 + (keysym - XKB_KEY_0));

    case XKB_KEY_parenright:
      return ui::VKEY_0;
    case XKB_KEY_exclam:
      return ui::VKEY_1;
    case XKB_KEY_at:
      return ui::VKEY_2;
    case XKB_KEY_numbersign:
      return ui::VKEY_3;
    case XKB_KEY_dollar:
      return ui::VKEY_4;
    case XKB_KEY_percent:
      return ui::VKEY_5;
    case XKB_KEY_asciicircum:
      return ui::VKEY_6;
    case XKB_KEY_ampersand:
      return ui::VKEY_7;
    case XKB_KEY_asterisk:
      return ui::VKEY_8;
    case XKB_KEY_parenleft:
      return ui::VKEY_9;

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
      return static_cast<ui::KeyboardCode>(
                 ui::VKEY_NUMPAD0 + (keysym - XKB_KEY_KP_0));

    case XKB_KEY_multiply:
    case XKB_KEY_KP_Multiply:
      return ui::VKEY_MULTIPLY;
    case XKB_KEY_KP_Add:
      return ui::VKEY_ADD;
    case XKB_KEY_KP_Separator:
      return ui::VKEY_SEPARATOR;
    case XKB_KEY_KP_Subtract:
      return ui::VKEY_SUBTRACT;
    case XKB_KEY_KP_Decimal:
      return ui::VKEY_DECIMAL;
    case XKB_KEY_KP_Divide:
      return ui::VKEY_DIVIDE;
    case XKB_KEY_KP_Equal:
    case XKB_KEY_equal:
    case XKB_KEY_plus:
      return ui::VKEY_OEM_PLUS;
    case XKB_KEY_comma:
    case XKB_KEY_less:
      return ui::VKEY_OEM_COMMA;
    case XKB_KEY_minus:
    case XKB_KEY_underscore:
      return ui::VKEY_OEM_MINUS;
    case XKB_KEY_greater:
    case XKB_KEY_period:
      return ui::VKEY_OEM_PERIOD;
    case XKB_KEY_colon:
    case XKB_KEY_semicolon:
      return ui::VKEY_OEM_1;
    case XKB_KEY_question:
    case XKB_KEY_slash:
      return ui::VKEY_OEM_2;
    case XKB_KEY_asciitilde:
    case XKB_KEY_grave:
      return ui::VKEY_OEM_3;
    case XKB_KEY_bracketleft:
    case XKB_KEY_braceleft:
      return ui::VKEY_OEM_4;
    case XKB_KEY_backslash:
    case XKB_KEY_bar:
      return ui::VKEY_OEM_5;
    case XKB_KEY_bracketright:
    case XKB_KEY_braceright:
      return ui::VKEY_OEM_6;
    case XKB_KEY_quotedbl:
    case XKB_KEY_apostrophe:
      return ui::VKEY_OEM_7;
    case XKB_KEY_Shift_L:
    case XKB_KEY_Shift_R:
      return ui::VKEY_SHIFT;
    case XKB_KEY_Control_L:
    case XKB_KEY_Control_R:
      return ui::VKEY_CONTROL;
    case XKB_KEY_Meta_L:
    case XKB_KEY_Meta_R:
    case XKB_KEY_Alt_L:
    case XKB_KEY_Alt_R:
      return ui::VKEY_MENU;
    case XKB_KEY_Pause:
      return ui::VKEY_PAUSE;
    case XKB_KEY_Caps_Lock:
      return ui::VKEY_CAPITAL;
    case XKB_KEY_Num_Lock:
      return ui::VKEY_NUMLOCK;
    case XKB_KEY_Scroll_Lock:
      return ui::VKEY_SCROLL;
    case XKB_KEY_Select:
      return ui::VKEY_SELECT;
    case XKB_KEY_Print:
      return ui::VKEY_PRINT;
    case XKB_KEY_Execute:
      return ui::VKEY_EXECUTE;
    case XKB_KEY_Insert:
    case XKB_KEY_KP_Insert:
      return ui::VKEY_INSERT;
    case XKB_KEY_Help:
      return ui::VKEY_HELP;
    case XKB_KEY_Super_L:
      return ui::VKEY_LWIN;
    case XKB_KEY_Super_R:
      return ui::VKEY_RWIN;
    case XKB_KEY_Menu:
      return ui::VKEY_APPS;
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
      return static_cast<ui::KeyboardCode>(ui::VKEY_F1 + (keysym - XKB_KEY_F1));
    case XKB_KEY_KP_F1:
    case XKB_KEY_KP_F2:
    case XKB_KEY_KP_F3:
    case XKB_KEY_KP_F4:
      return static_cast<ui::KeyboardCode>(
                 ui::VKEY_F1 + (keysym - XKB_KEY_KP_F1));

    case XKB_KEY_guillemotleft:
    case XKB_KEY_guillemotright:
    case XKB_KEY_degree:
    // In the case of canadian multilingual keyboard layout, VKEY_OEM_102 is
    // assigned to ugrave key.
    case XKB_KEY_ugrave:
    case XKB_KEY_Ugrave:
      return ui::VKEY_OEM_102;  // international backslash key in 102 keyboard.
  }
  DLOG(WARNING) << "Unknown keysym: " << base::StringPrintf("0x%x", keysym);
  return ui::VKEY_UNKNOWN;
}

}  // namespace ozonewayland
