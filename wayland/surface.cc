// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/surface.h"
#include "ozone/wayland/display.h"

namespace ui {
static const struct wl_callback_listener frameListener = {
    WaylandSurface::surfaceFrameCallback
};

WaylandSurface::WaylandSurface()
    : surface_(0)
    , frameCallBack_(0)
    , id_(0)
{
  static WaylandSurfaceId bufferHandleId = 0;
  WaylandDisplay* display = WaylandDisplay::GetDisplay();
  surface_ = wl_compositor_create_surface(display->GetCompositor());

  if (!surface_)
    return;

  bufferHandleId++;
  id_ = bufferHandleId;
}

WaylandSurface::~WaylandSurface()
{
  id_ = 0;
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
  // TODO(kalyan): Add support for synchronization.
  /*int ret = 0;
  while (frameCallBack_ && ret != -1)
    ret = WaylandDisplay::GetDisplay()->SyncDisplay();*/

  return 0;
}

void WaylandSurface::surfaceFrameCallback(void* data, struct wl_callback* callback, uint32_t)
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


