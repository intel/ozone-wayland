// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_GFX_OZONE_DISPLAY_H_
#define OZONE_UI_GFX_OZONE_DISPLAY_H_

#include "ozone/platform/ozone_export_wayland.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gfx/ozone/surface_factory_ozone.h"

namespace gfx {

class OZONE_WAYLAND_EXPORT OzoneDisplay : public gfx::SurfaceFactoryOzone {
 public:
  OzoneDisplay();
  virtual ~OzoneDisplay();

  // Returns the static instance last set using SetInstance().
  static OzoneDisplay* GetInstance();

  // Sets the implementation delegate. Ownership is retained by the caller.
  static void SetInstance(OzoneDisplay* instance);

  virtual void LookAheadOutputGeometry();

  // Ozone Display implementation:
  virtual gfx::SurfaceFactoryOzone::HardwareState InitializeHardware() OVERRIDE;
  virtual void ShutdownHardware() OVERRIDE;
  virtual intptr_t GetNativeDisplay() OVERRIDE;

  virtual gfx::AcceleratedWidget GetAcceleratedWidget() OVERRIDE;

  // Ownership is passed to the caller.
  virtual scoped_ptr<gfx::SurfaceOzoneEGL> CreateEGLSurfaceForWidget(
        gfx::AcceleratedWidget widget) OVERRIDE;

  virtual bool LoadEGLGLES2Bindings(
    gfx::SurfaceFactoryOzone::AddGLLibraryCallback add_gl_library,
    gfx::SurfaceFactoryOzone::SetGLGetProcAddressProcCallback
        proc_address) OVERRIDE;
  virtual const int32* GetEGLSurfaceProperties(
      const int32* desired_list) OVERRIDE;

 private:
  static OzoneDisplay* instance_;
  DISALLOW_COPY_AND_ASSIGN(OzoneDisplay);
};

}  // namespace gfx

#endif  // OZONE_UI_GFX_OZONE_DISPLAY_H_
