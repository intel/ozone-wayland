// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/input_device.h"

#include "ozone/wayland/input_method_event_filter.h"
#include "ozone/wayland/keyboard.h"
#include "ozone/wayland/pointer.h"

namespace ui {

WaylandInputDevice::WaylandInputDevice(WaylandDisplay* display, uint32_t id)
  :input_keyboard_(NULL),
   input_pointer_(NULL),
   input_method_filter_(NULL)

{
  static const struct wl_seat_listener kInputSeatListener = {
    WaylandInputDevice::OnSeatCapabilities,
  };

  input_seat_ = static_cast<wl_seat*>(
      wl_registry_bind(display->registry(), id, &wl_seat_interface, 1));
  wl_seat_add_listener(input_seat_, &kInputSeatListener, this);
  wl_seat_set_user_data(input_seat_, this);
}

WaylandInputDevice::~WaylandInputDevice()
{
  if (input_keyboard_)
    delete input_keyboard_;

  if (input_pointer_)
    delete input_pointer_;

  if (input_seat_)
    wl_seat_destroy(input_seat_);

  if (input_method_filter_)
    delete input_method_filter_;
}

void WaylandInputDevice::OnSeatCapabilities(void *data, wl_seat *seat, uint32_t caps)
{
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

InputMethod* WaylandInputDevice::GetInputMethod() const
{
  if (!input_method_filter_)
    input_method_filter_ = new WaylandInputMethodEventFilter;

  input_method_filter_->GetInputMethod();
}

}  // namespace ui
