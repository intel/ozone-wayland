// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_INPUT_DEVICE_H_
#define OZONE_WAYLAND_INPUT_DEVICE_H_

#include <wayland-client.h>
#include "base/basictypes.h"

namespace ozonewayland {

class WaylandKeyboard;
class WaylandPointer;
class WaylandDisplay;
class WaylandWindow;

class WaylandInputDevice {
 public:
  WaylandInputDevice(WaylandDisplay* display, uint32_t id);
  ~WaylandInputDevice();

  wl_seat* GetInputSeat() const { return input_seat_; }
  WaylandKeyboard* GetKeyBoard() const { return input_keyboard_; }
  WaylandPointer* GetPointer() const { return input_pointer_; }
  unsigned GetFocusWindowHandle() const { return focused_window_handle_; }
  void SetFocusWindowHandle(unsigned windowhandle);

 private:
  static void OnSeatCapabilities(void *data,
                                 wl_seat *seat,
                                 uint32_t caps);

  // Keeps track of current focused window.
  unsigned focused_window_handle_;
  wl_seat* input_seat_;
  WaylandKeyboard* input_keyboard_;
  WaylandPointer* input_pointer_;

  DISALLOW_COPY_AND_ASSIGN(WaylandInputDevice);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_INPUT_DEVICE_H_
