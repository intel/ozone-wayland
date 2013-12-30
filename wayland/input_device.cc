// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/input_device.h"

#include "ozone/wayland/display.h"
#include "ozone/wayland/input/keyboard.h"
#include "ozone/wayland/input/pointer.h"

namespace ozonewayland {

WaylandInputDevice::WaylandInputDevice(WaylandDisplay* display, uint32_t id)
    : focused_window_handle_(0),
      grab_window_handle_(0),
      grab_button_(0),
      input_keyboard_(NULL),
      input_pointer_(NULL) {
  static const struct wl_seat_listener kInputSeatListener = {
    WaylandInputDevice::OnSeatCapabilities,
  };

  input_seat_ = static_cast<wl_seat*>(
      wl_registry_bind(display->registry(), id, &wl_seat_interface, 1));
  wl_seat_add_listener(input_seat_, &kInputSeatListener, this);
  wl_seat_set_user_data(input_seat_, this);
}

WaylandInputDevice::~WaylandInputDevice() {
  if (input_keyboard_)
    delete input_keyboard_;

  if (input_pointer_)
    delete input_pointer_;

  if (input_seat_)
    wl_seat_destroy(input_seat_);
}

void WaylandInputDevice::OnSeatCapabilities(void *data,
                                            wl_seat *seat,
                                            uint32_t caps) {
  WaylandInputDevice* device = static_cast<WaylandInputDevice*>(data);
  if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !device->input_keyboard_) {
    device->input_keyboard_ = new WaylandKeyboard();
  } else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && device->input_keyboard_) {
    delete device->input_keyboard_;
    device->input_keyboard_ = NULL;
  }

  if ((caps & WL_SEAT_CAPABILITY_POINTER) && !device->input_pointer_) {
    device->input_pointer_ = new WaylandPointer();
  } else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && device->input_pointer_) {
    delete device->input_pointer_;
    device->input_pointer_ = NULL;
  }

  if (device->input_keyboard_)
    device->input_keyboard_->OnSeatCapabilities(seat, caps);

  if (device->input_pointer_)
    device->input_pointer_->OnSeatCapabilities(seat, caps);
}

void WaylandInputDevice::SetFocusWindowHandle(unsigned windowhandle) {
  focused_window_handle_ = windowhandle;
}

void WaylandInputDevice::SetGrabWindowHandle(unsigned windowhandle,
                                             uint32_t button) {
  grab_window_handle_ = windowhandle;
  grab_button_ = button;
}

}  // namespace ozonewayland
