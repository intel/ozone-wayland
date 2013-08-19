// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland_screen.h"
#include <wayland-client.h>
#include "ozone/wayland_display.h"

namespace ui {

WaylandScreen::WaylandScreen(WaylandDisplay* display, uint32_t id)
    : output_(NULL)
{
  static const wl_output_listener kOutputListener = {
    WaylandScreen::OutputHandleGeometry,
    WaylandScreen::OutputHandleMode,
  };

  output_ = static_cast<wl_output*>(
      wl_registry_bind(display->registry(), id, &wl_output_interface, 1));
  wl_output_add_listener(output_, &kOutputListener, this);
}

WaylandScreen::~WaylandScreen()
{
  if (output_)
    wl_output_destroy(output_);
}

gfx::Rect WaylandScreen::Geometry() const
{
  return rect_;
}

// static
void WaylandScreen::OutputHandleGeometry(void *data,
    wl_output *output, int32_t x, int32_t y,
    int32_t physical_width, int32_t physical_height,
    int32_t subpixel, const char* make,
    const char* model, int32_t output_transform)
{
  WaylandScreen* screen = static_cast<WaylandScreen*>(data);
  gfx::Point point = gfx::Point(x, y);
  screen->rect_.set_origin(point);
}

// static
void WaylandScreen::OutputHandleMode(void* data,
    wl_output* wl_output, uint32_t flags,
    int32_t width, int32_t height,
    int32_t refresh)
{
  WaylandScreen* screen = static_cast<WaylandScreen*>(data);
  if (flags & WL_OUTPUT_MODE_CURRENT) {
      screen->rect_.set_width(width);
      screen->rect_.set_height(height);
      screen->refresh_ = refresh;
  }
}

}  // namespace ui
