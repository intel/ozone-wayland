// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/display.h"
#include "ozone/wayland/surface.h"
#include "ozone/wayland/display.h"

namespace ui {
static const struct wl_callback_listener frameListener = {
    WaylandSurface::surfaceFrameCallback
};

WaylandSurface::WaylandSurface()
    : surface_(0),
    frameCallBack_(0),
    m_queue(NULL)
{
  WaylandDisplay* display = WaylandDisplay::GetInstance();
  surface_ = wl_compositor_create_surface(display->GetCompositor());

  if (!surface_)
    return;

  m_queue = wl_display_create_queue(display->display());
  wl_proxy_set_queue((struct wl_proxy *)display->registry(), m_queue);
}

WaylandSurface::~WaylandSurface()
{
  deleteFrameCallBack();

  if (surface_) {
    wl_surface_destroy(surface_);
    surface_ = 0;
  }
}

void WaylandSurface::addFrameCallBack()
{
  if (frameCallBack_)
    return;

  frameCallBack_ = wl_surface_frame(surface_);
  wl_callback_add_listener(frameCallBack_, &frameListener, this);
  wl_proxy_set_queue((struct wl_proxy *)frameCallBack_, m_queue);
}

void WaylandSurface::deleteFrameCallBack()
{
  if (frameCallBack_) {
    wl_callback_destroy(frameCallBack_);
    frameCallBack_ = 0;
  }
}

int WaylandSurface::ensureFrameCallBackDone()
{
  if (!m_queue)
    return -1;

  int ret = 0;
  wl_display* display = WaylandDisplay::GetInstance()->display();

  if (frameCallBack_) {
    while (frameCallBack_ && ret != -1)
      ret = wl_display_dispatch_queue(display, m_queue);

    wl_display_dispatch_pending(display);
  }

  return ret;
}

void WaylandSurface::surfaceFrameCallback(void* data, struct wl_callback* callback,
                                          uint32_t)
{
  WaylandSurface* surface = static_cast<WaylandSurface *>(data);
  wl_callback_destroy(callback);
  surface->destroyFrameCallBack();
}

void WaylandSurface::destroyFrameCallBack()
{
  frameCallBack_ = 0;
}

}  // namespace ui
