// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/pointer.h"

#include "ozone/wayland/cursor.h"
#include "ozone/wayland/dispatcher.h"
#include "ui/base/events/event.h"
#include "ui/base/hit_test.h"

#include <linux/input.h>

namespace ozonewayland {

WaylandPointer::WaylandPointer()
  : cursor_(NULL),
    dispatcher_(NULL)
{
}

WaylandPointer::~WaylandPointer()
{
  if (cursor_) {
    delete cursor_;
    cursor_ = NULL;
  }
}

void WaylandPointer::OnSeatCapabilities(wl_seat *seat, uint32_t caps)
{
  static const struct wl_pointer_listener kInputPointerListener = {
    WaylandPointer::OnPointerEnter,
    WaylandPointer::OnPointerLeave,
    WaylandPointer::OnMotionNotify,
    WaylandPointer::OnButtonNotify,
    WaylandPointer::OnAxisNotify,
  };

  if (!cursor_)
   cursor_ = new WaylandCursor(WaylandDisplay::GetInstance()->shm());

  dispatcher_ = WaylandDispatcher::GetInstance();

  if ((caps & WL_SEAT_CAPABILITY_POINTER) && !cursor_->GetInputPointer()) {
    wl_pointer* input_pointer = wl_seat_get_pointer(seat);
      cursor_->SetInputPointer(input_pointer);
    wl_pointer_set_user_data(input_pointer, this);
    wl_pointer_add_listener(input_pointer, &kInputPointerListener, this);
  } else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && cursor_->GetInputPointer()) {
    wl_pointer_destroy(cursor_->GetInputPointer());
    cursor_->SetInputPointer(NULL);
  }
}

void WaylandPointer::OnMotionNotify(void* data,
    wl_pointer* input_pointer,
    uint32_t time,
    wl_fixed_t sx_w,
    wl_fixed_t sy_w)
{
  WaylandPointer* device = static_cast<WaylandPointer*>(data);
  float sx = wl_fixed_to_double(sx_w);
  float sy = wl_fixed_to_double(sy_w);

  device->pointer_position_.SetPoint(sx, sy);
  device->dispatcher_->MotionNotify(sx, sy);
}

void WaylandPointer::OnAxisNotify(void* data,
    wl_pointer* input_pointer,
    uint32_t time,
    uint32_t axis,
    int32_t value)
{
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
  }

  device->dispatcher_->AxisNotify(device->pointer_position_.x(),
                                  device->pointer_position_.y(), x_offset,
                                  y_offset);
}

void WaylandPointer::OnButtonNotify(void* data,
    wl_pointer* input_pointer,
    uint32_t serial,
    uint32_t time,
    uint32_t button,
    uint32_t state)
{
  WaylandPointer* device = static_cast<WaylandPointer*>(data);
  int currentState;
  if (state == WL_POINTER_BUTTON_STATE_PRESSED)
    currentState = 1;
  else
    currentState = 0;
  // TODO(vignatti): simultaneous clicks fail
  int flags = 0;
  if (button == BTN_LEFT)
    flags = ui::EF_LEFT_MOUSE_BUTTON;
  else if (button == BTN_RIGHT)
    flags = ui::EF_RIGHT_MOUSE_BUTTON;
  else if (button == BTN_MIDDLE)
    flags = ui::EF_MIDDLE_MOUSE_BUTTON;

  device->dispatcher_->ButtonNotify(currentState, flags,
                                    device->pointer_position_.x(),
                                    device->pointer_position_.y());
}

void WaylandPointer::OnPointerEnter(void* data,
    wl_pointer* input_pointer,
    uint32_t serial,
    wl_surface* surface,
    wl_fixed_t sx_w,
    wl_fixed_t sy_w)
{
  WaylandPointer* device = static_cast<WaylandPointer*>(data);
  // TODO(vignatti): sx and sy have to be used for setting different resizing
  // and other cursors.

  device->cursor_->Update(WaylandCursor::CURSOR_LEFT_PTR, serial);
  device->dispatcher_->PointerEnter(device->pointer_position_.x(),
                                    device->pointer_position_.y());
}

void WaylandPointer::OnPointerLeave(void* data,
    wl_pointer* input_pointer,
    uint32_t serial,
    wl_surface* surface)
{
  WaylandPointer* device = static_cast<WaylandPointer*>(data);
  device->dispatcher_->PointerLeave(device->pointer_position_.x(),
                                    device->pointer_position_.y());
}

}  // namespace ozonewayland
