// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_OZONE_DISPLAY_H_
#define OZONE_WAYLAND_OZONE_DISPLAY_H_

#include "base/compiler_specific.h"
#include "base/message_loop/message_loop.h"
#include "ui/base/ozone/surface_factory_ozone.h"

namespace ui {
class EventFactoryWayland;
class WaylandDisplay;
class WaylandWindow;
class WaylandDispatcher;
class WaylandScreen;
}

namespace OzoneWayland {

class OzoneProcessObserver;
class OzoneDisplayChannel;
class OzoneDisplayChannelHost;

class OzoneDisplay : public ui::SurfaceFactoryOzone,
                     public base::MessageLoop::DestructionObserver {
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
  virtual bool LoadEGLGLES2Bindings() OVERRIDE;
  // Returns true if resizing of eglwindow associated with opaque handle was
  // successful else returns false.
  virtual bool AttemptToResizeAcceleratedWidget(gfx::AcceleratedWidget w,
                                                const gfx::Rect& bounds) OVERRIDE;
  virtual gfx::VSyncProvider* GetVSyncProvider(gfx::AcceleratedWidget w) OVERRIDE;
  virtual bool SchedulePageFlip(gfx::AcceleratedWidget w) OVERRIDE;

  // MessageLoop::DestructionObserver overrides.
  virtual void WillDestroyCurrentMessageLoop() OVERRIDE;

 private:
  enum Launch {
   None = 0x00,
   SingleProcess = 0x01,
   MultiProcess = 0x02
  };

  enum Process {
   PreLaunch = 0x00,
   Browser = 0x01,
   Gpu = 0x02
  };

  enum State {
   UnInitialized = 0x00,
   Initialized = 0x01,
   PendingOutPut = 0x02,
   ChannelConnected = 0x04
  };

  typedef unsigned LaunchType;
  typedef unsigned ProcessType;
  typedef unsigned CurrentState;

  void EstablishChannel(unsigned id);
  void OnChannelEstablished(unsigned id);
  void OnChannelClosed(unsigned id);
  void OnChannelHostDestroyed();
  void OnOutputSizeChanged(ui::WaylandScreen* screen, int width, int height);
  void OnOutputSizeChanged(unsigned width, unsigned height);

  void Terminate();
  void ValidateLaunchType();

  LaunchType launch_type_;
  ProcessType process_type_;
  CurrentState state_;
  ui::SurfaceFactoryOzone::HardwareState initialized_state_;
  bool initialized_ :1;

  ui::WaylandDispatcher* dispatcher_;
  ui::WaylandDisplay* display_;
  ui::WaylandWindow* root_window_;
  OzoneProcessObserver* child_process_observer_;
  OzoneDisplayChannel* channel_;
  OzoneDisplayChannelHost* host_;
  ui::EventFactoryWayland* e_factory_;
  char* spec_;
  static OzoneDisplay* instance_;

  friend class OzoneProcessObserver;
  friend class OzoneDisplayChannelHost;
  friend class OzoneDisplayChannel;
  friend class ui::WaylandScreen;
  DISALLOW_COPY_AND_ASSIGN(OzoneDisplay);
};

}  // namespace OzoneWayland

#endif  // OZONE_WAYLAND_OZONE_DISPLAY_H_
