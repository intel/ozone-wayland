// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/egl/surface_ozone_wayland.h"

#include "ozone/wayland/display.h"
#include "ozone/wayland/window.h"
#include "ui/gfx/vsync_provider.h"

namespace ozonewayland {

SurfaceOzoneWayland::SurfaceOzoneWayland(unsigned handle)
    : handle_(handle) {
}

SurfaceOzoneWayland::~SurfaceOzoneWayland() {
  WaylandDisplay::GetInstance()->DestroyWindow(handle_);
  WaylandDisplay::GetInstance()->FlushDisplay();
}

intptr_t SurfaceOzoneWayland::GetNativeWindow() {
  return WaylandDisplay::GetInstance()->GetNativeWindow(handle_);
}

bool SurfaceOzoneWayland::ResizeNativeWindow(
    const gfx::Size& viewport_size) {
  WaylandWindow* window = WaylandDisplay::GetInstance()->GetWindow(handle_);
  DCHECK(window);
  window->Resize(viewport_size.width(), viewport_size.height());
  return true;
}

bool SurfaceOzoneWayland::OnSwapBuffers() {
  return true;
}

void SurfaceOzoneWayland::OnSwapBuffersAsync(
    const ui::SwapCompletionCallback& callback) {
}

scoped_ptr<gfx::VSyncProvider> SurfaceOzoneWayland::CreateVSyncProvider() {
  return scoped_ptr<gfx::VSyncProvider>();
}

}  // namespace ozonewayland
