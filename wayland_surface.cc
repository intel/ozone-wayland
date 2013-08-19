// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland_surface.h"
#include "ozone/wayland_display.h"

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
  display->addPendingTask();
}

WaylandSurface::~WaylandSurface()
{
  id_ = 0;
  deleteFrameCallBack();

  if (surface_) {
    wl_surface_destroy(surface_);
    surface_ = 0;
  }

  WaylandDisplay::GetDisplay()->addPendingTask();
}

void WaylandSurface::addFrameCallBack()
{
  if (frameCallBack_)
    return;

  frameCallBack_ = wl_surface_frame(surface_);
  wl_callback_add_listener(frameCallBack_, &frameListener, this);
  WaylandDisplay::GetDisplay()->addPendingTask();
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
  int ret = 0;
  while (frameCallBack_ && ret != -1)
    ret = WaylandDisplay::GetDisplay()->SyncDisplay();

  return ret;
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


