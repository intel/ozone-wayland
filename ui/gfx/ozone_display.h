// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_GFX_OZONE_DISPLAY_H_
#define OZONE_UI_GFX_OZONE_DISPLAY_H_

#include "ozone/platform/ozone_export_wayland.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/ozone/public/surface_factory_ozone.h"

namespace ui {

class OZONE_WAYLAND_EXPORT OzoneDisplay : public SurfaceFactoryOzone {
 public:
  OzoneDisplay();
  virtual ~OzoneDisplay();

  // Returns the static instance last set using SetInstance().
  static OzoneDisplay* GetInstance();

  // Sets the implementation delegate. Ownership is retained by the caller.
  static void SetInstance(OzoneDisplay* instance);

  virtual void LookAheadOutputGeometry();

  // Ozone Display implementation:
  virtual intptr_t GetNativeDisplay() OVERRIDE;

  virtual gfx::AcceleratedWidget GetAcceleratedWidget() OVERRIDE;

  // Ownership is passed to the caller.
  virtual scoped_ptr<SurfaceOzoneEGL> CreateEGLSurfaceForWidget(
        gfx::AcceleratedWidget widget) OVERRIDE;

  virtual bool LoadEGLGLES2Bindings(
    SurfaceFactoryOzone::AddGLLibraryCallback add_gl_library,
    SurfaceFactoryOzone::SetGLGetProcAddressProcCallback
        proc_address) OVERRIDE;
  virtual const int32* GetEGLSurfaceProperties(
      const int32* desired_list) OVERRIDE;

 private:
  static OzoneDisplay* instance_;
  DISALLOW_COPY_AND_ASSIGN(OzoneDisplay);
};

}  // namespace ui

#endif  // OZONE_UI_GFX_OZONE_DISPLAY_H_
