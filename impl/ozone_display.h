// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_IMPL_OZONE_DISPLAY_H_
#define OZONE_IMPL_OZONE_DISPLAY_H_

#include "base/compiler_specific.h"
#include "ozone/platform/ozone_export_wayland.h"
#include "ozone/ui/events/output_change_observer.h"
#include "ui/gfx/ozone/surface_factory_ozone.h"

namespace ozonewayland {

class DesktopScreenWayland;
class EventConverterOzoneWayland;
class OzoneDisplayChannel;
class OzoneDisplayChannelHost;
class WaylandDisplay;
class WaylandWindow;
class WindowChangeObserver;

class OZONE_WAYLAND_EXPORT OzoneDisplay
    : public gfx::SurfaceFactoryOzone,
      public OutputChangeObserver {
 public:
  static OzoneDisplay* GetInstance();

  OzoneDisplay();
  virtual ~OzoneDisplay();

  virtual const char* DefaultDisplaySpec() OVERRIDE;
  virtual gfx::Screen* CreateDesktopScreen() OVERRIDE;
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

  // OutputChangeObserver overrides.
  virtual void OnOutputSizeChanged(unsigned width, unsigned height) OVERRIDE;

  const DesktopScreenWayland* GetPrimaryScreen() const;

 private:
  WaylandWindow* GetWidget(gfx::AcceleratedWidget w);

  void Terminate();
  void InitializeDispatcher(int fd = 0);
  void LookAheadOutputGeometry();

  static void DelayedInitialization(OzoneDisplay* display);

  bool initialized_;
  gfx::SurfaceFactoryOzone::HardwareState initialized_state_;
  unsigned last_realized_widget_handle_;
  const int kMaxDisplaySize_;

  DesktopScreenWayland* desktop_screen_;
  WaylandDisplay* display_;
  OzoneDisplayChannel* channel_;
  OzoneDisplayChannelHost* host_;
  EventConverterOzoneWayland* event_converter_;
  char* spec_;
  static OzoneDisplay* instance_;
  DISALLOW_COPY_AND_ASSIGN(OzoneDisplay);
};

}  // namespace ozonewayland

#endif  // OZONE_IMPL_OZONE_DISPLAY_H_
