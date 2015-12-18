// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_EGL_SURFACE_OZONE_WAYLAND
#define OZONE_WAYLAND_EGL_SURFACE_OZONE_WAYLAND

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "ui/gfx/gfx_export.h"
#include "ui/ozone/public/surface_ozone_egl.h"

namespace ozonewayland {

// Provides EGL support for SurfaceOzone.
class SurfaceOzoneWayland : public ui::SurfaceOzoneEGL {
 public:
  explicit SurfaceOzoneWayland(unsigned handle);
  ~SurfaceOzoneWayland() override;

  // SurfaceOzone:
  intptr_t GetNativeWindow() override;
  bool ResizeNativeWindow(const gfx::Size& viewport_size) override;
  bool OnSwapBuffers() override;
  void OnSwapBuffersAsync(const ui::SwapCompletionCallback& callback) override;
  scoped_ptr<gfx::VSyncProvider> CreateVSyncProvider() override;

 private:
  unsigned handle_;
  DISALLOW_COPY_AND_ASSIGN(SurfaceOzoneWayland);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_EGL_SURFACE_OZONE_WAYLAND
