// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_SURFACE_FACTORY_H_
#define OZONE_WAYLAND_SURFACE_FACTORY_H_

#include "base/compiler_specific.h"
#include "ui/base/ozone/surface_factory_ozone.h"

namespace ui {

class WaylandDisplay;
class EventFactoryWayland;

class SurfaceFactoryWayland : public SurfaceFactoryOzone {
 public:
  SurfaceFactoryWayland();
  virtual ~SurfaceFactoryWayland();

  // ui::SurfaceFactoryOzone
  virtual const char* DefaultDisplaySpec() OVERRIDE;
  virtual intptr_t InitializeHardware() OVERRIDE;
  virtual void ShutdownHardware() OVERRIDE;
  virtual gfx::AcceleratedWidget GetAcceleratedWidget() OVERRIDE;
  virtual gfx::AcceleratedWidget RealizeAcceleratedWidget(
      gfx::AcceleratedWidget w) OVERRIDE;
  virtual bool LoadEGLGLES2Bindings() OVERRIDE;
  virtual bool AttemptToResizeAcceleratedWidget(gfx::AcceleratedWidget w, const gfx::Rect& bounds) OVERRIDE;
  virtual gfx::VSyncProvider* GetVSyncProvider(gfx::AcceleratedWidget w) OVERRIDE;

 private:
  void InitializeWaylandEvent();

  ui::EventFactoryWayland *e_factory;
  ui::WaylandDisplay* display_;

};

}  // namespace ui

#endif  // OZONE_WAYLAND_SURFACE_FACTORY_H_
