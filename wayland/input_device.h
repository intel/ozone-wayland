// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_INPUT_DEVICE_H_
#define OZONE_WAYLAND_INPUT_DEVICE_H_

#include "ozone/wayland/display.h"

namespace ui {

class Event;
class KeyBoard;
class Pointer;
class InputMethod;
class WaylandInputMethodEventFilter;

class WaylandInputDevice {
 public:
  WaylandInputDevice(WaylandDisplay* display, uint32_t id);
  ~WaylandInputDevice();

  wl_seat* GetInputSeat() { return input_seat_; }
  InputMethod* GetInputMethod() const;
  KeyBoard* GetKeyBoard() const { return input_keyboard_; }
  Pointer* GetPointer() const { return input_pointer_; }

 private:
  static void OnSeatCapabilities(
      void *data,
      wl_seat *seat,
      uint32_t caps);

  wl_seat* input_seat_;
  KeyBoard* input_keyboard_;
  Pointer* input_pointer_;
  mutable WaylandInputMethodEventFilter* input_method_filter_;

  DISALLOW_COPY_AND_ASSIGN(WaylandInputDevice);
};

}  // namespace ui

#endif  // OZONE_WAYLAND_INPUT_DEVICE_H_
