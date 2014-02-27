// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_IMPL_SURFACE_FACTORY_WAYLAND_H_
#define OZONE_IMPL_SURFACE_FACTORY_WAYLAND_H_

#include "ui/gfx/ozone/surface_factory_ozone.h"

namespace ozonewayland {

class SurfaceFactoryWayland : public gfx::SurfaceFactoryOzone {
 public:
  static SurfaceFactoryWayland* GetInstance();

  SurfaceFactoryWayland();
  virtual ~SurfaceFactoryWayland();

  virtual SurfaceFactoryOzone::HardwareState InitializeHardware() OVERRIDE;
  virtual intptr_t GetNativeDisplay() OVERRIDE;
  virtual void ShutdownHardware() OVERRIDE;
  // GetAcceleratedWidget return's a opaque handle associated with a accelerated
  // widget.
  virtual gfx::AcceleratedWidget GetAcceleratedWidget() OVERRIDE;
  // RealizeAcceleratedWidget takes opaque handle as parameter and returns
  // eglwindow assosicated with it.
  virtual gfx::AcceleratedWidget RealizeAcceleratedWidget(
    gfx::AcceleratedWidget w) OVERRIDE;
  virtual bool LoadEGLGLES2Bindings(
    gfx::SurfaceFactoryOzone::AddGLLibraryCallback add_gl_library,
    gfx::SurfaceFactoryOzone::SetGLGetProcAddressProcCallback
        proc_address) OVERRIDE;
  // Returns true if resizing of eglwindow associated with opaque handle was
  // successful else returns false.
  virtual bool AttemptToResizeAcceleratedWidget(
      gfx::AcceleratedWidget w, const gfx::Rect& bounds) OVERRIDE;
  virtual scoped_ptr<gfx::VSyncProvider> CreateVSyncProvider(
      gfx::AcceleratedWidget w) OVERRIDE;
  virtual bool SchedulePageFlip(gfx::AcceleratedWidget w) OVERRIDE;
  virtual const int32* GetEGLSurfaceProperties(
      const int32* desired_list) OVERRIDE;

 private:
  bool initialized_;
  gfx::SurfaceFactoryOzone::HardwareState initialized_state_;
  unsigned last_realized_widget_handle_;

  DISALLOW_COPY_AND_ASSIGN(SurfaceFactoryWayland);
};

}  // namespace ozonewayland

#endif  // OZONE_IMPL_SURFACE_FACTORY_WAYLAND_H_
