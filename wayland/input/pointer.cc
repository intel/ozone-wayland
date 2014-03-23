// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/input/pointer.h"

#include <linux/input.h>

#include "ozone/ui/events/event_factory_ozone_wayland.h"
#include "ozone/wayland/input/cursor.h"
#include "ozone/wayland/input_device.h"
#include "ozone/wayland/window.h"
#include "ui/events/event.h"

namespace ozonewayland {

WaylandPointer::WaylandPointer()
  : cursor_(NULL),
    dispatcher_(NULL),
    pointer_position_(0, 0) {
}

WaylandPointer::~WaylandPointer() {
  delete cursor_;
}

void WaylandPointer::OnSeatCapabilities(wl_seat *seat, uint32_t caps) {
  static const struct wl_pointer_listener kInputPointerListener = {
    WaylandPointer::OnPointerEnter,
    WaylandPointer::OnPointerLeave,
    WaylandPointer::OnMotionNotify,
    WaylandPointer::OnButtonNotify,
    WaylandPointer::OnAxisNotify,
  };

  if (!cursor_)
    cursor_ = new WaylandCursor(WaylandDisplay::GetInstance()->shm());

  dispatcher_ = ui::EventFactoryOzoneWayland::GetInstance()->EventConverter();

  if ((caps & WL_SEAT_CAPABILITY_POINTER) && !cursor_->GetInputPointer()) {
    wl_pointer* input_pointer = wl_seat_get_pointer(seat);
      cursor_->SetInputPointer(input_pointer);
    wl_pointer_set_user_data(input_pointer, this);
    wl_pointer_add_listener(input_pointer, &kInputPointerListener, this);
  } else if (!(caps & WL_SEAT_CAPABILITY_POINTER)
                && cursor_->GetInputPointer()) {
    cursor_->SetInputPointer(NULL);
  }
}

void WaylandPointer::OnMotionNotify(void* data,
                                    wl_pointer* input_pointer,
                                    uint32_t time,
                                    wl_fixed_t sx_w,
                                    wl_fixed_t sy_w) {
  WaylandPointer* device = static_cast<WaylandPointer*>(data);
  WaylandInputDevice* input = WaylandDisplay::GetInstance()->PrimaryInput();
  float sx = wl_fixed_to_double(sx_w);
  float sy = wl_fixed_to_double(sy_w);

  device->pointer_position_.SetPoint(sx, sy);
  if (input->GetGrabWindowHandle() &&
        input->GetGrabWindowHandle() != input->GetFocusWindowHandle()) {
      return;
  }

  device->dispatcher_->MotionNotify(sx, sy);
}

void WaylandPointer::OnButtonNotify(void* data,
                                    wl_pointer* input_pointer,
                                    uint32_t serial,
                                    uint32_t time,
                                    uint32_t button,
                                    uint32_t state) {
  WaylandPointer* device = static_cast<WaylandPointer*>(data);
  WaylandDisplay::GetInstance()->SetSerial(serial);
  WaylandInputDevice* input = WaylandDisplay::GetInstance()->PrimaryInput();
  if (input->GetFocusWindowHandle() && input->GetGrabButton() == 0 &&
        state == WL_POINTER_BUTTON_STATE_PRESSED)
    input->SetGrabWindowHandle(input->GetFocusWindowHandle(), button);

  if (input->GetGrabWindowHandle()) {
    ui::EventType type = ui::ET_MOUSE_PRESSED;
    if (state == WL_POINTER_BUTTON_STATE_RELEASED)
      type = ui::ET_MOUSE_RELEASED;

    // TODO(vignatti): simultaneous clicks fail
    ui::EventFlags flags = ui::EF_NONE;
    if (button == BTN_LEFT)
      flags = ui::EF_LEFT_MOUSE_BUTTON;
    else if (button == BTN_RIGHT)
      flags = ui::EF_RIGHT_MOUSE_BUTTON;
    else if (button == BTN_MIDDLE)
      flags = ui::EF_MIDDLE_MOUSE_BUTTON;

    device->dispatcher_->ButtonNotify(input->GetFocusWindowHandle(),
                                      type,
                                      flags,
                                      device->pointer_position_.x(),
                                      device->pointer_position_.y());
  }

  if (input->GetGrabWindowHandle() && input->GetGrabButton() == button &&
        state == WL_POINTER_BUTTON_STATE_RELEASED)
    input->SetGrabWindowHandle(0, 0);
}

void WaylandPointer::OnAxisNotify(void* data,
                                  wl_pointer* input_pointer,
                                  uint32_t time,
                                  uint32_t axis,
                                  int32_t value) {
  int x_offset = 0, y_offset = 0;
  WaylandPointer* device = static_cast<WaylandPointer*>(data);
  const int delta = ui::MouseWheelEvent::kWheelDelta;

  switch (axis) {
    case WL_POINTER_AXIS_HORIZONTAL_SCROLL:
      x_offset = value > 0 ? -delta : delta;
      break;
    case WL_POINTER_AXIS_VERTICAL_SCROLL:
      y_offset = value > 0 ? -delta : delta;
      break;
    default:
      break;
  }

  device->dispatcher_->AxisNotify(device->pointer_position_.x(),
                                  device->pointer_position_.y(),
                                  x_offset,
                                  y_offset);
}

void WaylandPointer::OnPointerEnter(void* data,
                                    wl_pointer* input_pointer,
                                    uint32_t serial,
                                    wl_surface* surface,
                                    wl_fixed_t sx_w,
                                    wl_fixed_t sy_w) {
  WaylandInputDevice* input = WaylandDisplay::GetInstance()->PrimaryInput();

  if (!surface) {
    input->SetFocusWindowHandle(0);
    return;
  }

  WaylandPointer* device = static_cast<WaylandPointer*>(data);
  WaylandWindow* window =
      static_cast<WaylandWindow*>(wl_surface_get_user_data(surface));
  unsigned handle = window->Handle();
  float sx = wl_fixed_to_double(sx_w);
  float sy = wl_fixed_to_double(sy_w);

  WaylandDisplay::GetInstance()->SetSerial(serial);
  device->pointer_position_.SetPoint(sx, sy);
  input->SetFocusWindowHandle(handle);
  device->dispatcher_->PointerEnter(handle,
                                    device->pointer_position_.x(),
                                    device->pointer_position_.y());
}

void WaylandPointer::OnPointerLeave(void* data,
                                    wl_pointer* input_pointer,
                                    uint32_t serial,
                                    wl_surface* surface) {
  WaylandPointer* device = static_cast<WaylandPointer*>(data);
  WaylandDisplay::GetInstance()->SetSerial(serial);

  WaylandInputDevice* input = WaylandDisplay::GetInstance()->PrimaryInput();
  device->dispatcher_->PointerLeave(input->GetFocusWindowHandle(),
                                    device->pointer_position_.x(),
                                    device->pointer_position_.y());
  input->SetFocusWindowHandle(0);
}

}  // namespace ozonewayland
