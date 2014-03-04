// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_OZONE_HARDWARE_DISPLAY_H_
#define OZONE_WAYLAND_OZONE_HARDWARE_DISPLAY_H_

#include "ozone/ui/gfx/ozone_display.h"

namespace ozonewayland {

class WaylandDisplay;

class OzoneHardwareDisplay : public gfx::OzoneDisplay {
 public:
  OzoneHardwareDisplay();
  virtual ~OzoneHardwareDisplay();

  virtual gfx::SurfaceFactoryOzone::HardwareState InitializeHardware() OVERRIDE;
  virtual void ShutdownHardware() OVERRIDE;
  virtual intptr_t GetNativeDisplay() OVERRIDE;
  virtual void FlushDisplay() OVERRIDE;

  virtual gfx::AcceleratedWidget GetAcceleratedWidget() OVERRIDE;
  virtual gfx::AcceleratedWidget RealizeAcceleratedWidget(
      gfx::AcceleratedWidget w) OVERRIDE;
  virtual bool AttemptToResizeAcceleratedWidget(
      gfx::AcceleratedWidget w, const gfx::Rect& bounds) OVERRIDE;
  virtual void DestroyWidget(gfx::AcceleratedWidget w) OVERRIDE;
  virtual void LookAheadOutputGeometry() OVERRIDE;

 private:
  void Terminate();

  WaylandDisplay* native_display_;
  DISALLOW_COPY_AND_ASSIGN(OzoneHardwareDisplay);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_OZONE_HARDWARE_DISPLAY_H_
