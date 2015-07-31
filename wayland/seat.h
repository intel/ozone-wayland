// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_SEAT_H_
#define OZONE_WAYLAND_SEAT_H_

#include <wayland-client.h>
#include <vector>

#include "base/basictypes.h"
#include "ozone/ui/events/ime_state_change_handler.h"
#include "third_party/skia/include/core/SkBitmap.h"

namespace ozonewayland {

class WaylandKeyboard;
class WaylandPointer;
class WaylandDisplay;
class WaylandTouchscreen;
class WaylandTextInput;

class WaylandSeat : public ui::IMEStateChangeHandler {
 public:
  WaylandSeat(WaylandDisplay* display, uint32_t id);
  ~WaylandSeat() override;

  wl_seat* GetWLSeat() const { return seat_; }
  WaylandKeyboard* GetKeyBoard() const { return input_keyboard_; }
  WaylandPointer* GetPointer() const { return input_pointer_; }
  unsigned GetFocusWindowHandle() const { return focused_window_handle_; }
  unsigned GetGrabWindowHandle() const { return grab_window_handle_; }
  uint32_t GetGrabButton() const { return grab_button_; }
  void SetFocusWindowHandle(unsigned windowhandle);
  void SetGrabWindowHandle(unsigned windowhandle, uint32_t button);
  void SetCursorBitmap(const std::vector<SkBitmap>& bitmaps,
                       const gfx::Point& location);

  void ResetIme() override;
  void ImeCaretBoundsChanged(gfx::Rect rect) override;
  void ShowInputPanel() override;
  void HideInputPanel() override;

 private:
  static void OnSeatCapabilities(void *data,
                                 wl_seat *seat,
                                 uint32_t caps);

  // Keeps track of current focused window.
  unsigned focused_window_handle_;
  unsigned grab_window_handle_;
  uint32_t grab_button_;
  wl_seat* seat_;
  WaylandKeyboard* input_keyboard_;
  WaylandPointer* input_pointer_;
  WaylandTouchscreen* input_touch_;
  WaylandTextInput* text_input_;

  DISALLOW_COPY_AND_ASSIGN(WaylandSeat);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_SEAT_H_
