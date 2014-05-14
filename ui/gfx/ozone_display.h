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

  virtual void FlushDisplay() = 0;
  virtual gfx::AcceleratedWidget RealizeAcceleratedWidget(
      gfx::AcceleratedWidget w) = 0;
  virtual bool AttemptToResizeAcceleratedWidget(gfx::AcceleratedWidget w,
                                                const gfx::Size& bounds) = 0;
  virtual void DestroyWidget(gfx::AcceleratedWidget w) = 0;
  virtual void LookAheadOutputGeometry();

 private:
  static OzoneDisplay* instance_;
  DISALLOW_COPY_AND_ASSIGN(OzoneDisplay);
};

}  // namespace gfx

#endif  // OZONE_UI_GFX_OZONE_DISPLAY_H_
