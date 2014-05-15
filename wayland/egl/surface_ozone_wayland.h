// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_EGL_SURFACE_OZONE_WAYLAND
#define OZONE_WAYLAND_EGL_SURFACE_OZONE_WAYLAND

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "ui/gfx/gfx_export.h"
#include "ui/gfx/ozone/surface_ozone_egl.h"

namespace ozonewayland {

// Provides EGL support for SurfaceOzone.
class SurfaceOzoneWayland : public gfx::SurfaceOzoneEGL {
 public:
  explicit SurfaceOzoneWayland(unsigned handle);
  virtual ~SurfaceOzoneWayland() OVERRIDE;

  // SurfaceOzone:
  virtual intptr_t GetNativeWindow() OVERRIDE;
  virtual bool ResizeNativeWindow(const gfx::Size& viewport_size) OVERRIDE;
  virtual scoped_ptr<gfx::VSyncProvider> CreateVSyncProvider() OVERRIDE;

 private:
  unsigned handle_;
  DISALLOW_COPY_AND_ASSIGN(SurfaceOzoneWayland);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_EGL_SURFACE_OZONE_WAYLAND
