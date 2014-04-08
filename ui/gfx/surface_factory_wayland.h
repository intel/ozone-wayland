// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_GFX_SURFACE_FACTORY_WAYLAND_H_
#define OZONE_UI_GFX_SURFACE_FACTORY_WAYLAND_H_

#include "ui/gfx/ozone/surface_factory_ozone.h"

#include "ui/gfx/gfx_export.h"

namespace gfx {
class SurfaceOzoneEGL;
class SurfaceOzoneCanvas;

class GFX_EXPORT SurfaceFactoryWayland : public gfx::SurfaceFactoryOzone {
 public:
  SurfaceFactoryWayland();
  virtual ~SurfaceFactoryWayland();

  virtual SurfaceFactoryOzone::HardwareState InitializeHardware() OVERRIDE;
  virtual intptr_t GetNativeDisplay() OVERRIDE;
  virtual void ShutdownHardware() OVERRIDE;
  // GetAcceleratedWidget return's a opaque handle associated with a accelerated
  // widget.
  virtual gfx::AcceleratedWidget GetAcceleratedWidget() OVERRIDE;
  // Ownership is passed to the caller.
  virtual scoped_ptr<SurfaceOzoneEGL> CreateEGLSurfaceForWidget(
        gfx::AcceleratedWidget widget) OVERRIDE;

  virtual bool LoadEGLGLES2Bindings(
    gfx::SurfaceFactoryOzone::AddGLLibraryCallback add_gl_library,
    gfx::SurfaceFactoryOzone::SetGLGetProcAddressProcCallback
        proc_address) OVERRIDE;
  virtual const int32* GetEGLSurfaceProperties(
      const int32* desired_list) OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(SurfaceFactoryWayland);
};

}  // namespace gfx

#endif  // OZONE_UI_GFX_SURFACE_FACTORY_WAYLAND_H_
