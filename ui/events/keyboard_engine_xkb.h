// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_EVENTS_INPUT_KEYBOARD_ENGINE_XKB_H_
#define OZONE_UI_EVENTS_INPUT_KEYBOARD_ENGINE_XKB_H_

#include <xkbcommon/xkbcommon.h>

#include "base/basictypes.h"
#include "ozone/platform/ozone_export_wayland.h"
#include "ozone/ui/events/keyboard_codes_ozone.h"

namespace ui {

class KeyboardEngineXKB {
 public:
  KeyboardEngineXKB();
  ~KeyboardEngineXKB();

  void OnKeyboardKeymap(int fd, uint32_t size);
  void OnKeyModifiers(uint32_t mods_depressed,
                      uint32_t mods_latched,
                      uint32_t mods_locked,
                      uint32_t group);

  KeyboardCode KeyboardCodeFromNativeKeysym(unsigned hardwarecode);
  uint16 CharacterCodeFromNativeKeySym(unsigned sym,
                                       unsigned flags);

  uint32_t GetKeyBoardModifiers() const { return keyboard_modifiers_; }

 private:
  void InitXKB();
  void FiniXKB();
  bool IsOnlyCapsLocked() const;
  void NormalizeKey();
  unsigned ConvertKeyCodeFromEvdev(unsigned hardwarecode);
  uint16 EvaluateCharacterCodeFromNativeKeySym(unsigned hardwarecode,
                                               unsigned flags);

  // Keeps track of the currently active keyboard modifiers. We keep this
  // since we want to advertise keyboard modifiers with mouse events.
  uint32_t keyboard_modifiers_;
  uint32_t mods_depressed_;
  uint32_t mods_latched_;
  uint32_t mods_locked_;
  uint32_t group_;
  int last_key_;
  uint32_t last_modifiers_;
  xkb_keysym_t cached_sym_;
  KeyboardCode cached_keyboard_code_;
  uint16 cached_character_code_;

  // keymap used to transform keyboard events.
  struct xkb_keymap *keymap_;
  struct xkb_state *state_;
  struct xkb_context *context_;

  DISALLOW_COPY_AND_ASSIGN(KeyboardEngineXKB);
};

}  // namespace ui

#endif  // OZONE_UI_EVENTS_INPUT_KEYBOARD_ENGINE_XKB_H_
