// Copyright (c) 2014 Noser Engineering AG. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/input/touchscreen.h"

#include <linux/input.h>

#include "ozone/ui/events/event_factory_ozone_wayland.h"
#include "ozone/wayland/input/cursor.h"
#include "ozone/wayland/input_device.h"
#include "ozone/wayland/window.h"
#include "ui/events/event.h"

namespace ozonewayland {

WaylandTouchscreen::WaylandTouchscreen()
  : dispatcher_(NULL),
    pointer_position_(0, 0),
    wl_touch_(NULL) {
}

WaylandTouchscreen::~WaylandTouchscreen() {
  if (wl_touch_)
    wl_touch_destroy(wl_touch_);
}

void WaylandTouchscreen::OnSeatCapabilities(wl_seat *seat, uint32_t caps) {
  static const struct wl_touch_listener kInputTouchListener = {
    WaylandTouchscreen::OnTouchDown,
    WaylandTouchscreen::OnTouchUp,
    WaylandTouchscreen::OnTouchMotion,
    WaylandTouchscreen::OnTouchFrame,
    WaylandTouchscreen::OnTouchCancel,
  };

  dispatcher_ = ui::EventFactoryOzoneWayland::GetInstance()->EventConverter();

  if ((caps & WL_SEAT_CAPABILITY_TOUCH)) {
    wl_touch_ = wl_seat_get_touch(seat);
    wl_touch_set_user_data(wl_touch_, this);
    wl_touch_add_listener(wl_touch_, &kInputTouchListener, this);
  }
}

void WaylandTouchscreen::OnTouchDown(void *data,
                                     struct wl_touch *wl_touch,
                                     uint32_t serial,
                                     uint32_t time,
                                     struct wl_surface *surface,
                                     int32_t id,
                                     wl_fixed_t x,
                                     wl_fixed_t y) {
  WaylandTouchscreen* device = static_cast<WaylandTouchscreen*>(data);
  WaylandDisplay::GetInstance()->SetSerial(serial);
  WaylandInputDevice* input = WaylandDisplay::GetInstance()->PrimaryInput();

  // Need this code when the user clicks on a text input box directly
  if (!input->GetPointer()) {
    if (!surface) {
      input->SetFocusWindowHandle(0);
      return;
    }
    WaylandWindow* window =
         static_cast<WaylandWindow*>(wl_surface_get_user_data(surface));
    input->SetFocusWindowHandle(window->Handle());
  }

  if (input->GetFocusWindowHandle() && input->GetGrabButton() == 0)
    input->SetGrabWindowHandle(input->GetFocusWindowHandle(), id);

  float sx = wl_fixed_to_double(x);
  float sy = wl_fixed_to_double(y);

  device->pointer_position_.SetPoint(sx, sy);

  device->dispatcher_->TouchNotify(ui::ET_TOUCH_PRESSED, sx, sy, id, time);
}

void WaylandTouchscreen::OnTouchUp(void *data,
                                   struct wl_touch *wl_touch,
                                   uint32_t serial,
                                   uint32_t time,
                                   int32_t id) {
  WaylandTouchscreen* device = static_cast<WaylandTouchscreen*>(data);
  WaylandDisplay::GetInstance()->SetSerial(serial);
  WaylandInputDevice* input = WaylandDisplay::GetInstance()->PrimaryInput();

  device->dispatcher_->TouchNotify(ui::ET_TOUCH_RELEASED,
                                   device->pointer_position_.x(),
                                   device->pointer_position_.y(), id, time);

  if (input->GetGrabWindowHandle() && input->GetGrabButton() == id)
    input->SetGrabWindowHandle(0, 0);
}

void WaylandTouchscreen::OnTouchMotion(void *data,
                                      struct wl_touch *wl_touch,
                                      uint32_t time,
                                      int32_t id,
                                      wl_fixed_t x,
                                      wl_fixed_t y) {
  WaylandTouchscreen* device = static_cast<WaylandTouchscreen*>(data);
  WaylandInputDevice* input = WaylandDisplay::GetInstance()->PrimaryInput();
  float sx = wl_fixed_to_double(x);
  float sy = wl_fixed_to_double(y);

  device->pointer_position_.SetPoint(sx, sy);

  if (input->GetGrabWindowHandle() &&
    input->GetGrabWindowHandle() != input->GetFocusWindowHandle()) {
    return;
  }

  device->dispatcher_->TouchNotify(ui::ET_TOUCH_MOVED, sx, sy, id, time);
}

void WaylandTouchscreen::OnTouchFrame(void *data,
                                      struct wl_touch *wl_touch) {
  // TODO(speedpat): find out what should be done here
}

void WaylandTouchscreen::OnTouchCancel(void *data,
                                       struct wl_touch *wl_touch) {
  WaylandTouchscreen* device = static_cast<WaylandTouchscreen*>(data);
  WaylandInputDevice* input = WaylandDisplay::GetInstance()->PrimaryInput();

  device->dispatcher_->TouchNotify(ui::ET_TOUCH_CANCELLED,
                                   device->pointer_position_.x(),
                                   device->pointer_position_.y(),
                                   input->GetGrabButton(),
                                   0);

  if (input->GetGrabWindowHandle() && input->GetGrabButton() != 0)
    input->SetGrabWindowHandle(0, 0);
}

}  // namespace ozonewayland
