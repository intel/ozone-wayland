// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_INPUT_DEVICE_H_
#define OZONE_WAYLAND_INPUT_DEVICE_H_

#include "base/basictypes.h"
#include <wayland-client.h>

namespace ozonewayland {

class Event;
class WaylandKeyboard;
class WaylandPointer;
class InputMethod;
class WaylandInputMethodEventFilter;
class WaylandDisplay;

class WaylandInputDevice {
 public:
  WaylandInputDevice(WaylandDisplay* display, uint32_t id);
  ~WaylandInputDevice();

  wl_seat* GetInputSeat() { return input_seat_; }
  InputMethod* GetInputMethod() const;
  WaylandKeyboard* GetKeyBoard() const { return input_keyboard_; }
  WaylandPointer* GetPointer() const { return input_pointer_; }

 private:
  static void OnSeatCapabilities(
      void *data,
      wl_seat *seat,
      uint32_t caps);

  wl_seat* input_seat_;
  WaylandKeyboard* input_keyboard_;
  WaylandPointer* input_pointer_;
  mutable WaylandInputMethodEventFilter* input_method_filter_;

  DISALLOW_COPY_AND_ASSIGN(WaylandInputDevice);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_INPUT_DEVICE_H_
