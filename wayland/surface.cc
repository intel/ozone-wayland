// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/surface.h"

#include "ozone/wayland/display.h"

namespace ozonewayland {
static const struct wl_callback_listener frameListener = {
    WaylandSurface::SurfaceFrameCallback
};

WaylandSurface::WaylandSurface() : surface_(NULL),
    frameCallBack_(NULL),
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
  DeleteFrameCallBack();

  if (surface_) {
    wl_surface_destroy(surface_);
    surface_ = 0;
  }
}

void WaylandSurface::AddFrameCallBack()
{
  if (frameCallBack_)
    return;

  frameCallBack_ = wl_surface_frame(surface_);
  wl_callback_add_listener(frameCallBack_, &frameListener, this);
  wl_proxy_set_queue((struct wl_proxy *)frameCallBack_, m_queue);
}

void WaylandSurface::DeleteFrameCallBack()
{
  if (frameCallBack_) {
    wl_callback_destroy(frameCallBack_);
    frameCallBack_ = 0;
  }
}

int WaylandSurface::EnsureFrameCallBackDone()
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

void WaylandSurface::SurfaceFrameCallback(void* data,
                                          struct wl_callback* callback,
                                          uint32_t)
{
  WaylandSurface* surface = static_cast<WaylandSurface *>(data);
  wl_callback_destroy(callback);
  surface->DestroyFrameCallBack();
}

void WaylandSurface::DestroyFrameCallBack()
{
  frameCallBack_ = NULL;
}

}  // namespace ui
