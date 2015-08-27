// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/seat.h"

#include "base/logging.h"
#include "ozone/wayland/data_device.h"
#include "ozone/wayland/display.h"
#include "ozone/wayland/input/cursor.h"
#include "ozone/wayland/input/keyboard.h"
#include "ozone/wayland/input/pointer.h"
#include "ozone/wayland/input/text_input.h"
#include "ozone/wayland/input/touchscreen.h"

namespace ozonewayland {

WaylandSeat::WaylandSeat(WaylandDisplay* display,
                         uint32_t id)
    : focused_window_handle_(0),
      grab_window_handle_(0),
      grab_button_(0),
      seat_(NULL),
      input_keyboard_(NULL),
      input_pointer_(NULL),
      input_touch_(NULL),
      text_input_(NULL) {
  static const struct wl_seat_listener kInputSeatListener = {
    WaylandSeat::OnSeatCapabilities,
  };

  seat_ = static_cast<wl_seat*>(
      wl_registry_bind(display->registry(), id, &wl_seat_interface, 1));
  DCHECK(seat_);
  wl_seat_add_listener(seat_, &kInputSeatListener, this);
  wl_seat_set_user_data(seat_, this);

  data_device_ = new WaylandDataDevice(display, seat_);
  text_input_ = new WaylandTextInput(this);
}

WaylandSeat::~WaylandSeat() {
  delete data_device_;
  delete input_keyboard_;
  delete input_pointer_;
  delete text_input_;
  if (input_touch_ != NULL) {
    delete input_touch_;
  }
  wl_seat_destroy(seat_);
}

void WaylandSeat::OnSeatCapabilities(void *data, wl_seat *seat, uint32_t caps) {
  WaylandSeat* device = static_cast<WaylandSeat*>(data);
  if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !device->input_keyboard_) {
    device->input_keyboard_ = new WaylandKeyboard();
    device->input_keyboard_->OnSeatCapabilities(seat, caps);
  } else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && device->input_keyboard_) {
    delete device->input_keyboard_;
    device->input_keyboard_ = NULL;
  }

  if ((caps & WL_SEAT_CAPABILITY_POINTER) && !device->input_pointer_) {
    device->input_pointer_ = new WaylandPointer();
    device->input_pointer_->OnSeatCapabilities(seat, caps);
  } else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && device->input_pointer_) {
    delete device->input_pointer_;
    device->input_pointer_ = NULL;
  }

  if ((caps & WL_SEAT_CAPABILITY_TOUCH) && !device->input_touch_) {
    device->input_touch_ = new WaylandTouchscreen();
    device->input_touch_->OnSeatCapabilities(seat, caps);
  } else if (!(caps & WL_SEAT_CAPABILITY_TOUCH) && device->input_touch_) {
    delete device->input_touch_;
    device->input_touch_ = NULL;
  }
}

void WaylandSeat::SetFocusWindowHandle(unsigned windowhandle) {
  focused_window_handle_ = windowhandle;
  WaylandWindow* window = NULL;
  if (windowhandle)
    window = WaylandDisplay::GetInstance()->GetWindow(windowhandle);
  text_input_->SetActiveWindow(window);
}

void WaylandSeat::SetGrabWindowHandle(unsigned windowhandle, uint32_t button) {
  grab_window_handle_ = windowhandle;
  grab_button_ = button;
}

void WaylandSeat::SetCursorBitmap(const std::vector<SkBitmap>& bitmaps,
                                  const gfx::Point& location) {
  if (!input_pointer_) {
    LOG(WARNING) << "Tried to change cursor without input configured";
    return;
  }
  input_pointer_->Cursor()->UpdateBitmap(
      bitmaps, location, WaylandDisplay::GetInstance()->GetSerial());
}

void WaylandSeat::MoveCursor(const gfx::Point& location) {
  if (!input_pointer_) {
    LOG(WARNING) << "Tried to move cursor without input configured";
    return;
  }

  input_pointer_->Cursor()->MoveCursor(
      location, WaylandDisplay::GetInstance()->GetSerial());
}

void WaylandSeat::ResetIme() {
  text_input_->ResetIme();
}

void WaylandSeat::ImeCaretBoundsChanged(gfx::Rect rect) {
  NOTIMPLEMENTED();
}

void WaylandSeat::ShowInputPanel() {
  text_input_->ShowInputPanel(seat_);
}

void WaylandSeat::HideInputPanel() {
  text_input_->HideInputPanel(seat_);
}

}  // namespace ozonewayland
