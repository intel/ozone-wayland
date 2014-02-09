// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_INPUT_DEVICE_H_
#define OZONE_WAYLAND_INPUT_DEVICE_H_

#include <wayland-client.h>
#include "base/basictypes.h"
#include "ozone/ui/ime/ime_state_change_handler.h"

namespace ozonewayland {

class WaylandKeyboard;
class WaylandPointer;
class WaylandDisplay;

class WaylandInputDevice : public IMEStateChangeHandler {
 public:
  WaylandInputDevice(WaylandDisplay* display, uint32_t id);
  virtual ~WaylandInputDevice();

  wl_seat* GetInputSeat() const { return input_seat_; }
  WaylandKeyboard* GetKeyBoard() const { return input_keyboard_; }
  WaylandPointer* GetPointer() const { return input_pointer_; }
  unsigned GetFocusWindowHandle() const { return focused_window_handle_; }
  unsigned GetGrabWindowHandle() const { return grab_window_handle_; }
  uint32_t GetGrabButton() const { return grab_button_; }
  void SetFocusWindowHandle(unsigned windowhandle);
  void SetGrabWindowHandle(unsigned windowhandle, uint32_t button);

  virtual void ResetIme() OVERRIDE;
  virtual void ImeCaretBoundsChanged(gfx::Rect rect) OVERRIDE;

 private:
  static void OnSeatCapabilities(void *data,
                                 wl_seat *seat,
                                 uint32_t caps);

  // Keeps track of current focused window.
  unsigned focused_window_handle_;
  unsigned grab_window_handle_;
  uint32_t grab_button_;
  wl_seat* input_seat_;
  WaylandKeyboard* input_keyboard_;
  WaylandPointer* input_pointer_;

  DISALLOW_COPY_AND_ASSIGN(WaylandInputDevice);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_INPUT_DEVICE_H_
