// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_GFX_SURFACE_OZONE_IMPL_EGL_H_
#define OZONE_UI_GFX_SURFACE_OZONE_IMPL_EGL_H_

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "ui/gfx/gfx_export.h"
#include "ui/gfx/ozone/surface_ozone_egl.h"

namespace gfx {

// Provides EGL support for SurfaceOzone.
class GFX_EXPORT SurfaceOzoneImplEGL : public SurfaceOzoneEGL {
 public:
  explicit SurfaceOzoneImplEGL(unsigned handle);
  virtual ~SurfaceOzoneImplEGL() OVERRIDE;

  // SurfaceOzone:
  virtual intptr_t GetNativeWindow() OVERRIDE;
  virtual bool ResizeNativeWindow(const gfx::Size& viewport_size) OVERRIDE;
  virtual scoped_ptr<gfx::VSyncProvider> CreateVSyncProvider() OVERRIDE;

 private:
  unsigned handle_;
  DISALLOW_COPY_AND_ASSIGN(SurfaceOzoneImplEGL);
};

}  // namespace gfx

#endif  // OZONE_UI_GFX_SURFACE_OZONE_IMPL_EGL_H_
