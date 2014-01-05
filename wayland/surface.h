// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_SURFACE_H_
#define OZONE_WAYLAND_SURFACE_H_

#include <wayland-client.h>

namespace ozonewayland {

class WaylandDisplay;

class WaylandSurface {
 public:
  WaylandSurface();
  ~WaylandSurface();
  struct wl_surface* wlSurface() const { return surface_; }

  // FrameCallBack.
  // Example usage to swap buffers
  // if (m_surface->ensureFrameCallBackDone() == -1)
  //    return;
  // m_surface->addFrameCallBack();
  // Swap buffers.
  void AddFrameCallBack();
  // Ensure deleteFrameCallBack(in case a framecallback is requested)
  // is called before destroying any EGL resources associated with the
  // surface. Example usage:
  // deleteFrameCallBack();
  // destroy egl window etc
  // m_surface = nullptr; i.e destroy WaylandSurface.
  void DeleteFrameCallBack();
  // see addFrameCallBack.
  int EnsureFrameCallBackDone();

  // callback
  static void SurfaceFrameCallback(void*, struct wl_callback*, uint32_t);

 private:
  void DestroyFrameCallBack();
  struct wl_surface* surface_;
  struct wl_callback* frameCallBack_;
  struct wl_event_queue* m_queue;
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_DISPLAY_H_
