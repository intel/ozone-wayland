// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/events/keyboard_code_conversion_ozone.h"

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "ui/events/event_constants.h"

namespace ozonewayland {

ui::KeyboardCode KeyboardCodeFromNativeKeysym(unsigned keysym) {
  if (keysym >= OZONECHARCODE_a && keysym <= OZONECHARCODE_z) {
    return static_cast<ui::KeyboardCode>(
               OZONECHARCODE_A + (keysym - OZONECHARCODE_a));
  }

  // Check if it's an alphabet or number we can directly cast it in this case.
  if ((keysym >= OZONECHARCODE_A && keysym <= OZONECHARCODE_Z) ||
       (keysym >= OZONECHARCODE_0 && keysym <= OZONECHARCODE_9))
    return static_cast<ui::KeyboardCode>(keysym);

  if (keysym >= OZONEACTIONKEY_BACK && keysym <= OZONEACTIONKEY_OEM_CLEAR)
    return static_cast<ui::KeyboardCode>(keysym - OZONEACTIONKEY_START);

  if (keysym >= OZONECHARCODE_FILE_SEPARATOR &&
       keysym <= OZONECHARCODE_TRADEMARK)
    return ui::VKEY_OEM_102;

  // Empty KeyCode.
  if (keysym == OZONECHARCODE_NULL)
    return ui::VKEY_UNKNOWN;

  DLOG(WARNING) << "Unknown keysym: " << base::StringPrintf("0x%x", keysym);
  return ui::VKEY_UNKNOWN;
}

uint16 CharacterCodeFromNativeKeySym(unsigned sym, unsigned flags) {
  const bool ctrl = (flags & ui::EF_CONTROL_DOWN) != 0;

  // Other ctrl characters
  if (ctrl) {
    unsigned keysym = sym;
    if (keysym >= OZONECHARCODE_a && keysym <= OZONECHARCODE_z)
      keysym = OZONECHARCODE_A + (keysym - OZONECHARCODE_a);

    if (keysym >= OZONECHARCODE_A && keysym <= OZONECHARCODE_Z)
      return keysym - OZONECHARCODE_A + 1;

    if ((flags & ui::EF_SHIFT_DOWN) != 0) {
      // following graphics chars require shift key to input.
      switch (keysym) {
        // ctrl-@ maps to \x00 (Null byte)
        case OZONECHARCODE_AT:
          return OZONECHARCODE_NULL;
        // ctrl-^ maps to \x1E (Record separator, Information separator two)
        case OZONECHARCODE_CARET_CIRCUMFLEX:
          return OZONECHARCODE_RECORD_SEPARATOR;
        // ctrl-_ maps to \x1F (Unit separator, Information separator one)
        case OZONECHARCODE_MINUS:
        case OZONECHARCODE_UNDER_SCORE:
          return OZONECHARCODE_UNIT_SEPARATOR;
        default:
          return OZONECHARCODE_NULL;
      }
    } else {
      switch (keysym) {
        // ctrl-[ maps to \x1B (Escape)
        case OZONECHARCODE_LEFT_BRACKET:
          return OZONECHARCODE_ESCAPE;
        // ctrl-\ maps to \x1C (File separator, Information separator four)
        case OZONECHARCODE_BACK_SLASH:
          return OZONECHARCODE_FILE_SEPARATOR;
        // ctrl-] maps to \x1D (Group separator, Information separator three)
        case OZONECHARCODE_RIGHT_BRACKET:
          return OZONECHARCODE_GROUP_SEPARATOR;
        // ctrl-Enter maps to \x0A (Line feed)
        case OZONECHARCODE_RETURN:
          return OZONECHARCODE_LINEFEED;
        default:
          return OZONECHARCODE_NULL;
      }
    }
  }

  if ((sym >= OZONECHARCODE_a && sym <= OZONECHARCODE_z) ||
       (sym >= OZONECHARCODE_A && sym <= OZONECHARCODE_Z) ||
         (sym >= OZONECHARCODE_0 && sym <= OZONECHARCODE_9))
    return sym;

  if (sym >= OZONEACTIONKEY_BACK && sym <= OZONEACTIONKEY_OEM_CLEAR) {
    switch (sym) {
      case OZONEACTIONKEY_TAB:
        return OZONECHARCODE_TAB;
      case OZONEACTIONKEY_ESCAPE:
        return OZONECHARCODE_ESCAPE;
      case OZONEACTIONKEY_RETURN:
        return OZONECHARCODE_RETURN;
      case OZONEACTIONKEY_SPACE:
        return OZONECHARCODE_SPACE;
      case OZONEACTIONKEY_BACK:
        return OZONECHARCODE_BACKSPACE;
        break;
      default:
        return OZONECHARCODE_NULL;
    }
  }

  return sym;
}

}  // namespace ozonewayland
