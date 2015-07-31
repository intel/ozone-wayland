// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_INPUT_KEYBOARD_H_
#define OZONE_WAYLAND_INPUT_KEYBOARD_H_

#include "ozone/wayland/display.h"

namespace ozonewayland {

class WaylandKeyboard {
 public:
  WaylandKeyboard();
  ~WaylandKeyboard();

  void OnSeatCapabilities(wl_seat *seat, uint32_t caps);

 private:
  static void OnKeyNotify(void* data,
                          wl_keyboard* input_keyboard,
                          uint32_t serial,
                          uint32_t time,
                          uint32_t key,
                          uint32_t state);

  static void OnKeyboardKeymap(void *data,
                               struct wl_keyboard *keyboard,
                               uint32_t format,
                               int fd,
                               uint32_t size);

  static void OnKeyboardEnter(void* data,
                              wl_keyboard* input_keyboard,
                              uint32_t serial,
                              wl_surface* surface,
                              wl_array* keys);

  static void OnKeyboardLeave(void* data,
                              wl_keyboard* input_keyboard,
                              uint32_t serial,
                              wl_surface* surface);

  static void OnKeyModifiers(void *data,
                             wl_keyboard *keyboard,
                             uint32_t serial,
                             uint32_t mods_depressed,
                             uint32_t mods_latched,
                             uint32_t mods_locked,
                             uint32_t group);

  wl_keyboard* input_keyboard_;
  WaylandDisplay* dispatcher_;

  DISALLOW_COPY_AND_ASSIGN(WaylandKeyboard);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_INPUT_KEYBOARD_H_
