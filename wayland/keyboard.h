// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_KEYBOARD_H_
#define OZONE_WAYLAND_KEYBOARD_H_

#include <xkbcommon/xkbcommon.h>
#include "ozone/wayland/display.h"
#include "base/memory/scoped_ptr.h"

namespace ui {

class Event;

class WaylandKeyboard {
 public:
  WaylandKeyboard();
  ~WaylandKeyboard();

  void OnSeatCapabilities(wl_seat *seat, uint32_t caps);

 private:
  static void DispatchEventHelper(scoped_ptr<ui::Event> key);
  static void DispatchEvent(scoped_ptr<ui::Event> event);

  static void OnKeyNotify(
      void* data,
      wl_keyboard* input_keyboard,
      uint32_t serial,
      uint32_t time,
      uint32_t key,
      uint32_t state);

  static void OnKeyboardKeymap(
      void *data,
      struct wl_keyboard *keyboard,
      uint32_t format,
      int fd,
      uint32_t size);

  static void OnKeyboardEnter(
      void* data,
      wl_keyboard* input_keyboard,
      uint32_t serial,
      wl_surface* surface,
      wl_array* keys);

  static void OnKeyboardLeave(
      void* data,
      wl_keyboard* input_keyboard,
      uint32_t serial,
      wl_surface* surface);

  static void OnKeyModifiers(
      void *data,
      wl_keyboard *keyboard,
      uint32_t serial,
      uint32_t mods_depressed,
      uint32_t mods_latched,
      uint32_t mods_locked,
      uint32_t group);

  wl_keyboard* input_keyboard_;

  // Keeps track of the currently active keyboard modifiers. We keep this
  // since we want to advertise keyboard modifiers with mouse events.
  uint32_t keyboard_modifiers_;

  // keymap used to transform keyboard events.
  struct xkb {
    struct xkb_rule_names names;
    struct xkb_keymap *keymap;
    struct xkb_state *state;
    struct xkb_context *context;
    xkb_mod_mask_t control_mask;
    xkb_mod_mask_t alt_mask;
    xkb_mod_mask_t shift_mask;
  };

  xkb xkb_;

  void InitXKB();
  void FiniXKB();

  DISALLOW_COPY_AND_ASSIGN(WaylandKeyboard);
};

}  // namespace ui

#endif  // OZONE_WAYLAND_KEYBOARD_H_
