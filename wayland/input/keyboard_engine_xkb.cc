// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/input/keyboard_engine_xkb.h"

#include <sys/mman.h>

#include "ozone/ui/events/keyboard_codes_ozone.h"
#include "ui/events/event.h"

namespace ozonewayland {

KeyboardEngineXKB::KeyboardEngineXKB() : keyboard_modifiers_(0),
    mods_depressed_(0),
    mods_latched_(0),
    mods_locked_(0),
    group_(0),
    last_key_(-1),
    cached_sym_(XKB_KEY_NoSymbol),
    keymap_(NULL),
    state_(NULL),
    context_(NULL) {
}

KeyboardEngineXKB::~KeyboardEngineXKB() {
  FiniXKB();
}

void KeyboardEngineXKB::OnKeyboardKeymap(int fd, uint32_t size) {
  char *map_str =
      reinterpret_cast<char*>(mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0));
  if (map_str == MAP_FAILED)
    return;

  InitXKB();
  keymap_ = xkb_map_new_from_string(context_,
                                    map_str,
                                    XKB_KEYMAP_FORMAT_TEXT_V1,
                                    (xkb_map_compile_flags)0);
  munmap(map_str, size);
  if (!keymap_)
    return;

  state_ = xkb_state_new(keymap_);
  if (state_) {
    xkb_map_unref(keymap_);
    keymap_ = NULL;
  }
}

void KeyboardEngineXKB::OnKeyModifiers(uint32_t mods_depressed,
                                        uint32_t mods_latched,
                                        uint32_t mods_locked,
                                        uint32_t group) {
  if (!state_)
    return;

  if ((mods_depressed_ == mods_depressed) && (mods_locked_ == mods_locked)
      && (mods_latched_ == mods_latched) && (group_ == group)) {
    return;
  }

  mods_depressed_ = mods_depressed;
  mods_locked_ = mods_locked;
  mods_latched_ = mods_latched;
  group_ = group;
  xkb_state_update_mask(state_,
                        mods_depressed_,
                        mods_latched_,
                        mods_locked_,
                        0,
                        0,
                        group_);

  keyboard_modifiers_ = 0;
  if (xkb_state_mod_name_is_active(
      state_, XKB_MOD_NAME_SHIFT, XKB_STATE_MODS_EFFECTIVE))
    keyboard_modifiers_ |= ui::EF_SHIFT_DOWN;

  if (xkb_state_mod_name_is_active(
      state_, XKB_MOD_NAME_CTRL, XKB_STATE_MODS_EFFECTIVE))
    keyboard_modifiers_ |= ui::EF_CONTROL_DOWN;

  if (xkb_state_mod_name_is_active(
      state_, XKB_MOD_NAME_ALT, XKB_STATE_MODS_EFFECTIVE))
    keyboard_modifiers_ |= ui::EF_ALT_DOWN;

  if (xkb_state_mod_name_is_active(
      state_, XKB_MOD_NAME_CAPS, XKB_STATE_MODS_EFFECTIVE))
    keyboard_modifiers_ |= ui::EF_CAPS_LOCK_DOWN;
}

unsigned KeyboardEngineXKB::ConvertKeyCodeFromEvdev(unsigned hardwarecode) {
  if (hardwarecode == last_key_)
    return cached_sym_;

  const xkb_keysym_t *syms;
  xkb_keysym_t sym;
  uint32_t code = hardwarecode + 8;
  uint32_t num_syms = xkb_key_get_syms(state_, code, &syms);
  if (num_syms == 1)
    sym = syms[0];
  else
    sym = XKB_KEY_NoSymbol;

  last_key_ = hardwarecode;
  cached_sym_ = sym;
  NormalizeKey();

  return cached_sym_;
}

bool KeyboardEngineXKB::IgnoreKeyNotify(
         unsigned hardwarecode, bool pressed) {
  // If the key is pressed or it's a special modifier key i.e altgr, we cannot
  // ignore it.
  // TODO(kalyan): Handle all needed cases here.
  if (pressed || IsSpecialModifier(hardwarecode))
    return false;

  // No modifiers set, we don't have to deal with any special cases. Ignore the
  // release events.
  if (!keyboard_modifiers_ || IsOnlyCapsLocked())
    return true;

  return false;
}

void KeyboardEngineXKB::InitXKB() {
  if (context_)
    return;

  context_ = xkb_context_new((xkb_context_flags)0);
}

void KeyboardEngineXKB::FiniXKB() {
  if (state_) {
    xkb_state_unref(state_);
    state_ = NULL;
  }

  if (keymap_) {
    xkb_map_unref(keymap_);
    keymap_ = NULL;
  }

  if (context_) {
    xkb_context_unref(context_);
    context_ = NULL;
  }
}

bool KeyboardEngineXKB::IsSpecialModifier(unsigned hardwarecode) {
    switch (ConvertKeyCodeFromEvdev(hardwarecode)) {
    case XKB_KEY_ISO_Level3_Shift:  // altgr
      return true;
    break;
    default:
    return false;
  }
}

bool KeyboardEngineXKB::IsOnlyCapsLocked() const {
  if ((keyboard_modifiers_ & ui::EF_CONTROL_DOWN) != 0)
    return false;

  if ((keyboard_modifiers_ & ui::EF_ALT_DOWN) != 0)
    return false;

  if ((keyboard_modifiers_ & ui::EF_SHIFT_DOWN) != 0)
    return false;

  return true;
}

void KeyboardEngineXKB::NormalizeKey() {
  if ((cached_sym_ >= XKB_KEY_A && cached_sym_ <= XKB_KEY_Z) ||
       (cached_sym_ >= XKB_KEY_a && cached_sym_ <= XKB_KEY_z) ||
         (cached_sym_ >= XKB_KEY_0 && cached_sym_ <= XKB_KEY_9))
    return;

  if (cached_sym_ >= XKB_KEY_KP_0 && cached_sym_ <= XKB_KEY_KP_9) {
    // Numpad Number-keys can be represented by a keysym value of 0-9 nos.
    cached_sym_ = XKB_KEY_0 + (cached_sym_ - XKB_KEY_KP_0);
  } else if (cached_sym_ > 0x01000100 && cached_sym_ < 0x01ffffff) {
    // Any UCS character in this range will simply be the character's
    // Unicode number plus 0x01000000.
    cached_sym_ = cached_sym_ - 0x001000000;
  } else if (cached_sym_ >= XKB_KEY_F1 && cached_sym_ <= XKB_KEY_F24) {
    cached_sym_ = ui::OZONEACTIONKEY_F1 + (cached_sym_ - XKB_KEY_F1);
  } else if (cached_sym_ >= XKB_KEY_KP_F1 && cached_sym_ <= XKB_KEY_KP_F4) {
      cached_sym_ = ui::OZONEACTIONKEY_F1 + (cached_sym_ - XKB_KEY_KP_F1);
  } else {
      switch (cached_sym_) {
        case XKB_KEY_dead_circumflex:
          cached_sym_ = ui::OZONECHARCODE_CARET_CIRCUMFLEX;
          break;
        case XKB_KEY_dead_diaeresis:
          cached_sym_ = ui::OZONECHARCODE_SPACING_DIAERESIS;
          break;
        case XKB_KEY_dead_perispomeni:
          cached_sym_ = ui::OZONECHARCODE_TILDE;
          break;
        case XKB_KEY_dead_acute:
          cached_sym_ = ui::OZONECHARCODE_SPACING_ACUTE;
          break;
        case XKB_KEY_dead_grave:
          cached_sym_ = ui::OZONECHARCODE_GRAVE_ASSCENT;
          break;
        case XKB_KEY_endash:
          cached_sym_ = ui::OZONECHARCODE_ENDASH;
          break;
        case XKB_KEY_singlelowquotemark:
          cached_sym_ = ui::OZONECHARCODE_SINGLE_LOW_QUOTATION_MARK;
          break;
        case XKB_KEY_dead_cedilla:
          cached_sym_ = ui::OZONECHARCODE_SPACING_CEDILLA;
          break;
        case XKB_KEY_KP_Equal:
          cached_sym_ = ui::OZONECHARCODE_EQUAL;
          break;
        case XKB_KEY_KP_Multiply:
          cached_sym_ = ui::OZONECHARCODE_MULTIPLY;
          break;
        case XKB_KEY_KP_Add:
          cached_sym_ = ui::OZONECHARCODE_PLUS;
          break;
        case XKB_KEY_KP_Separator:
          cached_sym_ = ui::OZONECHARCODE_COMMA;
          break;
        case XKB_KEY_KP_Subtract:
          cached_sym_ = ui::OZONECHARCODE_MINUS;
          break;
        case XKB_KEY_KP_Decimal:
          cached_sym_ = ui::OZONECHARCODE_PERIOD;
          break;
        case XKB_KEY_KP_Divide:
          cached_sym_ = ui::OZONECHARCODE_DIVISION;
          break;
        case XKB_KEY_Delete:
        case XKB_KEY_KP_Delete:
          cached_sym_ = ui::OZONEACTIONKEY_DELETE;
          break;
        case XKB_KEY_KP_Tab:
        case XKB_KEY_ISO_Left_Tab:
        case XKB_KEY_Tab:
        case XKB_KEY_3270_BackTab:
          cached_sym_ = ui::OZONEACTIONKEY_TAB;
          break;
        case XKB_KEY_Sys_Req:
        case XKB_KEY_Escape:
          cached_sym_ = ui::OZONEACTIONKEY_ESCAPE;
          break;
        case XKB_KEY_Linefeed:
          cached_sym_ = ui::OZONECHARCODE_LINEFEED;
          break;
        case XKB_KEY_Return:
        case XKB_KEY_KP_Enter:
        case XKB_KEY_ISO_Enter:
          cached_sym_ = ui::OZONEACTIONKEY_RETURN;
          break;
        case XKB_KEY_KP_Space:
        case XKB_KEY_space:
          cached_sym_ = ui::OZONEACTIONKEY_SPACE;
          break;
        case XKB_KEY_dead_caron:
          cached_sym_ = ui::OZONECHARCODE_CARON;
          break;
        case XKB_KEY_BackSpace:
          cached_sym_ = ui::OZONEACTIONKEY_BACK;
          break;
        case XKB_KEY_dead_doubleacute:
          cached_sym_ = ui::OZONECHARCODE_DOUBLE_ACUTE_ACCENT;
          break;
        case XKB_KEY_dead_horn:
          cached_sym_ = ui::OZONECHARCODE_COMBINING_HORN;
          break;
        case XKB_KEY_oe:
          cached_sym_ = ui::OZONECHARCODE_LSMALL_OE;
          break;
        case XKB_KEY_OE:
          cached_sym_ = ui::OZONECHARCODE_LOE;
          break;
        case XKB_KEY_idotless:
          cached_sym_ = ui::OZONECHARCODE_LSMALL_DOT_LESS_I;
          break;
        case XKB_KEY_kra:
          cached_sym_ = ui::OZONECHARCODE_LSMALL_KRA;
          break;
        case XKB_KEY_dead_stroke:
          cached_sym_ = ui::OZONECHARCODE_MINUS;
          break;
        case XKB_KEY_eng:
          cached_sym_ = ui::OZONECHARCODE_LSMALL_ENG;
          break;
        case XKB_KEY_ENG:
          cached_sym_ = ui::OZONECHARCODE_LENG;
          break;
        case XKB_KEY_leftsinglequotemark:
          cached_sym_ = ui::OZONECHARCODE_LEFT_SINGLE_QUOTATION_MARK;
          break;
        case XKB_KEY_rightsinglequotemark:
          cached_sym_ = ui::OZONECHARCODE_RIGHT_SINGLE_QUOTATION_MARK;
          break;
        case XKB_KEY_dead_belowdot:
          cached_sym_ = ui::OZONECHARCODE_COMBINING_DOT_BELOW;
          break;
        case XKB_KEY_dead_belowdiaeresis:
          cached_sym_ = ui::OZONECHARCODE_COMBINING_DIAERESIS_BELOW;
          break;
        case XKB_KEY_Clear:
        case XKB_KEY_KP_Begin:
          cached_sym_ = ui::OZONEACTIONKEY_CLEAR;
          break;
        case XKB_KEY_Home:
        case XKB_KEY_KP_Home:
          cached_sym_ = ui::OZONEACTIONKEY_HOME;
          break;
        case XKB_KEY_End:
        case XKB_KEY_KP_End:
          cached_sym_ = ui::OZONEACTIONKEY_END;
          break;
        case XKB_KEY_Page_Up:
        case XKB_KEY_KP_Page_Up:  // aka XKB_KEY_KP_Prior
          cached_sym_ = ui::OZONEACTIONKEY_PRIOR;
          break;
        case XKB_KEY_Page_Down:
        case XKB_KEY_KP_Page_Down:  // aka XKB_KEY_KP_Next
          cached_sym_ = ui::OZONEACTIONKEY_NEXT;
          break;
        case XKB_KEY_Left:
        case XKB_KEY_KP_Left:
          cached_sym_ = ui::OZONEACTIONKEY_LEFT;
          break;
        case XKB_KEY_Right:
        case XKB_KEY_KP_Right:
          cached_sym_ = ui::OZONEACTIONKEY_RIGHT;
          break;
        case XKB_KEY_Down:
        case XKB_KEY_KP_Down:
          cached_sym_ = ui::OZONEACTIONKEY_DOWN;
          break;
        case XKB_KEY_Up:
        case XKB_KEY_KP_Up:
          cached_sym_ = ui::OZONEACTIONKEY_UP;
          break;
        case XKB_KEY_Kana_Lock:
        case XKB_KEY_Kana_Shift:
          cached_sym_ = ui::OZONEACTIONKEY_KANA;
          break;
        case XKB_KEY_Hangul:
          cached_sym_ = ui::OZONEACTIONKEY_HANGUL;
          break;
        case XKB_KEY_Hangul_Hanja:
          cached_sym_ = ui::OZONEACTIONKEY_HANJA;
          break;
        case XKB_KEY_Kanji:
          cached_sym_ = ui::OZONEACTIONKEY_KANJI;
          break;
        case XKB_KEY_Henkan:
          cached_sym_ = ui::OZONEACTIONKEY_CONVERT;
          break;
        case XKB_KEY_Muhenkan:
          cached_sym_ = ui::OZONEACTIONKEY_NONCONVERT;
          break;
        case XKB_KEY_Zenkaku_Hankaku:
          cached_sym_ = ui::OZONEACTIONKEY_DBE_DBCSCHAR;
          break;
        case XKB_KEY_ISO_Level5_Shift:
          cached_sym_ = ui::OZONEACTIONKEY_OEM_8;
          break;
        case XKB_KEY_Shift_L:
        case XKB_KEY_Shift_R:
          cached_sym_ = ui::OZONEACTIONKEY_SHIFT;
          break;
        case XKB_KEY_Control_L:
        case XKB_KEY_Control_R:
          cached_sym_ = ui::OZONEACTIONKEY_CONTROL;
          break;
        case XKB_KEY_Meta_L:
        case XKB_KEY_Meta_R:
        case XKB_KEY_Alt_L:
        case XKB_KEY_Alt_R:
          cached_sym_ = ui::OZONEACTIONKEY_MENU;
          break;
        case XKB_KEY_ISO_Level3_Shift:
          cached_sym_ = ui::OZONEACTIONKEY_ALTGR;
          break;
        case XKB_KEY_Multi_key:
          cached_sym_ = 0xE6;
          break;
        case XKB_KEY_Pause:
          cached_sym_ = ui::OZONEACTIONKEY_PAUSE;
          break;
        case XKB_KEY_Caps_Lock:
          cached_sym_ = ui::OZONEACTIONKEY_CAPITAL;
          break;
        case XKB_KEY_Num_Lock:
          cached_sym_ = ui::OZONEACTIONKEY_NUMLOCK;
          break;
        case XKB_KEY_Scroll_Lock:
          cached_sym_ = ui::OZONEACTIONKEY_SCROLL;
          break;
        case XKB_KEY_Select:
          cached_sym_ = ui::OZONEACTIONKEY_SELECT;
          break;
        case XKB_KEY_Print:
          cached_sym_ = ui::OZONEACTIONKEY_PRINT;
          break;
        case XKB_KEY_Execute:
          cached_sym_ = ui::OZONEACTIONKEY_EXECUTE;
          break;
        case XKB_KEY_Insert:
        case XKB_KEY_KP_Insert:
          cached_sym_ = ui::OZONEACTIONKEY_INSERT;
          break;
        case XKB_KEY_Help:
          cached_sym_ = ui::OZONEACTIONKEY_HELP;
          break;
        case XKB_KEY_Super_L:
          cached_sym_ = ui::OZONEACTIONKEY_LWIN;
          break;
        case XKB_KEY_Super_R:
          cached_sym_ = ui::OZONEACTIONKEY_RWIN;
          break;
        case XKB_KEY_Menu:
          cached_sym_ = ui::OZONEACTIONKEY_APPS;
          break;
        case XKB_KEY_XF86Tools:
          cached_sym_ = ui::OZONEACTIONKEY_F13;
          break;
        case XKB_KEY_XF86Launch5:
          cached_sym_ = ui::OZONEACTIONKEY_F14;
          break;
        case XKB_KEY_XF86Launch6:
          cached_sym_ = ui::OZONEACTIONKEY_F15;
          break;
        case XKB_KEY_XF86Launch7:
          cached_sym_ = ui::OZONEACTIONKEY_F16;
          break;
        case XKB_KEY_XF86Launch8:
          cached_sym_ = ui::OZONEACTIONKEY_F17;
          break;
        case XKB_KEY_XF86Launch9:
          cached_sym_ = ui::OZONEACTIONKEY_F18;
          break;

        // For supporting multimedia buttons on a USB keyboard.
        case XKB_KEY_XF86Back:
          cached_sym_ = ui::OZONEACTIONKEY_BROWSER_BACK;
          break;
        case XKB_KEY_XF86Forward:
          cached_sym_ = ui::OZONEACTIONKEY_BROWSER_FORWARD;
          break;
        case XKB_KEY_XF86Reload:
          cached_sym_ = ui::OZONEACTIONKEY_BROWSER_REFRESH;
          break;
        case XKB_KEY_XF86Stop:
          cached_sym_ = ui::OZONEACTIONKEY_BROWSER_STOP;
          break;
        case XKB_KEY_XF86Search:
          cached_sym_ = ui::OZONEACTIONKEY_BROWSER_SEARCH;
          break;
        case XKB_KEY_XF86Favorites:
          cached_sym_ = ui::OZONEACTIONKEY_BROWSER_FAVORITES;
          break;
        case XKB_KEY_XF86HomePage:
          cached_sym_ = ui::OZONEACTIONKEY_BROWSER_HOME;
          break;
        case XKB_KEY_XF86AudioMute:
          cached_sym_ = ui::OZONEACTIONKEY_VOLUME_MUTE;
          break;
        case XKB_KEY_XF86AudioLowerVolume:
          cached_sym_ = ui::OZONEACTIONKEY_VOLUME_DOWN;
          break;
        case XKB_KEY_XF86AudioRaiseVolume:
          cached_sym_ = ui::OZONEACTIONKEY_VOLUME_UP;
          break;
        case XKB_KEY_XF86AudioNext:
          cached_sym_ = ui::OZONEACTIONKEY_MEDIA_NEXT_TRACK;
          break;
        case XKB_KEY_XF86AudioPrev:
          cached_sym_ = ui::OZONEACTIONKEY_MEDIA_PREV_TRACK;
          break;
        case XKB_KEY_XF86AudioStop:
          cached_sym_ = ui::OZONEACTIONKEY_MEDIA_STOP;
          break;
        case XKB_KEY_XF86AudioPlay:
          cached_sym_ = ui::OZONEACTIONKEY_MEDIA_PLAY_PAUSE;
          break;
        case XKB_KEY_XF86Mail:
          cached_sym_ = ui::OZONEACTIONKEY_MEDIA_LAUNCH_MAIL;
          break;
        case XKB_KEY_XF86LaunchA:
          cached_sym_ = ui::OZONEACTIONKEY_MEDIA_LAUNCH_APP1;
          break;
        case XKB_KEY_XF86LaunchB:
        case XKB_KEY_XF86Calculator:
          cached_sym_ = ui::OZONEACTIONKEY_MEDIA_LAUNCH_APP2;
          break;
        case XKB_KEY_XF86WLAN:
          cached_sym_ = ui::OZONEACTIONKEY_WLAN;
          break;
        case XKB_KEY_XF86PowerOff:
          cached_sym_ = ui::OZONEACTIONKEY_POWER;
          break;
        case XKB_KEY_XF86MonBrightnessDown:
          cached_sym_ = ui::OZONEACTIONKEY_BRIGHTNESS_DOWN;
          break;
        case XKB_KEY_XF86MonBrightnessUp:
          cached_sym_ = ui::OZONEACTIONKEY_BRIGHTNESS_UP;
          break;
        case XKB_KEY_XF86KbdBrightnessDown:
          cached_sym_ = ui::OZONEACTIONKEY_KBD_BRIGHTNESS_DOWN;
          break;
        case XKB_KEY_XF86KbdBrightnessUp:
          cached_sym_ = ui::OZONEACTIONKEY_KBD_BRIGHTNESS_UP;
          break;
        case XKB_KEY_emptyset:
        case XKB_KEY_NoSymbol:
          cached_sym_ = ui::OZONECHARCODE_NULL;
          break;
        default:
          break;
    }
  }
}

}  // namespace ozonewayland
