// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/gfx/surface_ozone_impl_egl.h"

#include "ozone/ui/gfx/ozone_display.h"
#include "ozone/ui/gfx/vsync_provider_wayland.h"

namespace gfx {

SurfaceOzoneImplEGL::SurfaceOzoneImplEGL(unsigned handle)
    : handle_(handle) {
}

SurfaceOzoneImplEGL::~SurfaceOzoneImplEGL() {
  OzoneDisplay::GetInstance()->DestroyWidget(handle_);
  OzoneDisplay::GetInstance()->FlushDisplay();
}

intptr_t SurfaceOzoneImplEGL::GetNativeWindow() {
  return OzoneDisplay::GetInstance()->RealizeAcceleratedWidget(handle_);
}

bool SurfaceOzoneImplEGL::ResizeNativeWindow(
    const gfx::Size& viewport_size) {
  return OzoneDisplay::GetInstance()->
      AttemptToResizeAcceleratedWidget(handle_, viewport_size);
}

scoped_ptr<gfx::VSyncProvider> SurfaceOzoneImplEGL::CreateVSyncProvider() {
  return scoped_ptr<gfx::VSyncProvider>(new WaylandSyncProvider());
}

}  // namespace gfx
