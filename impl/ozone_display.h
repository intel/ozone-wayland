// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_OZONE_DISPLAY_H_
#define OZONE_WAYLAND_OZONE_DISPLAY_H_

#include "base/compiler_specific.h"
#include "base/message_loop/message_loop.h"
#include "ui/gfx/ozone/surface_factory_ozone.h"

namespace ozonewayland {

class DesktopScreenWayland;
class EventFactoryWayland;
class OzoneProcessObserver;
class OzoneDisplayChannel;
class OzoneDisplayChannelHost;
class WaylandDisplay;
class WaylandWindow;
class WaylandDispatcher;
class WaylandScreen;
class WindowChangeObserver;

class OzoneDisplay : public gfx::SurfaceFactoryOzone,
                     public base::MessageLoop::DestructionObserver {
 public:
  enum {
    Create = 1, // Create a new Widget
    Show = 2, // Widget is visible.
    Hide = 3, // Widget is hidden.
    FullScreen = 4,  // Widget is in fullscreen mode.
    Maximized = 5, // Widget is maximized,
    Minimized = 6, // Widget is minimized.
    Restore = 7, // Restore Widget.
    Active = 8, // Widget is Activated.
    InActive = 9, // Widget is DeActivated.
    Resize = 10, // Widget is Resized.
    Destroyed = 11 // Widget is Destroyed.
  };

  typedef unsigned WidgetState;

  enum {
    Window = 1, // A decorated Window.
    WindowFrameLess = 2, // An undecorated Window.
    Menu = 3 // An undecorated Window, with transient properties
             // specialized to menus.
  };

  typedef unsigned WidgetType;

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
    gfx::SurfaceFactoryOzone::SetGLGetProcAddressProcCallback proc_address) OVERRIDE;
  // Returns true if resizing of eglwindow associated with opaque handle was
  // successful else returns false.
  virtual bool AttemptToResizeAcceleratedWidget(gfx::AcceleratedWidget w,
                                                const gfx::Rect& bounds) OVERRIDE;
  virtual gfx::VSyncProvider* GetVSyncProvider(gfx::AcceleratedWidget w) OVERRIDE;
  virtual bool SchedulePageFlip(gfx::AcceleratedWidget w) OVERRIDE;
  virtual const int32* GetEGLSurfaceProperties(const int32* desired_list) OVERRIDE;

  // MessageLoop::DestructionObserver overrides.
  virtual void WillDestroyCurrentMessageLoop() OVERRIDE;

  void SetWidgetState(gfx::AcceleratedWidget w,
                      WidgetState state,
                      unsigned width = 0,
                      unsigned height = 0);
  void OnWidgetStateChanged(gfx::AcceleratedWidget w,
                            WidgetState state,
                            unsigned width,
                            unsigned height);
  void SetWidgetTitle(gfx::AcceleratedWidget w, const string16& title);
  void OnWidgetTitleChanged(gfx::AcceleratedWidget w, const string16& title);

  void SetWidgetType(gfx::AcceleratedWidget w, WidgetType type);
  void OnWidgetTypeChanged(gfx::AcceleratedWidget w, WidgetType type);

  void SetWindowChangeObserver(WindowChangeObserver* observer);

 private:
  enum State {
   UnInitialized = 0x00,
   Initialized = 0x01,
   ChannelConnected = 0x02
  };

  typedef unsigned CurrentState;

  void EstablishChannel();
  void OnChannelEstablished();
  void OnChannelClosed();
  void OnChannelHostDestroyed();
  void OnOutputSizeChanged(WaylandScreen* screen, int width, int height);
  void OnOutputSizeChanged(unsigned width, unsigned height);
  WaylandWindow* CreateWidget(unsigned w);
  WaylandWindow* GetWidget(gfx::AcceleratedWidget w);

  void Terminate();
  void InitializeDispatcher(int fd = 0);
  void LookAheadOutputGeometry();

  CurrentState state_;
  gfx::SurfaceFactoryOzone::HardwareState initialized_state_;
  const int kMaxDisplaySize_;

  DesktopScreenWayland* desktop_screen_;
  WaylandDispatcher* dispatcher_;
  WaylandDisplay* display_;
  OzoneProcessObserver* child_process_observer_;
  OzoneDisplayChannel* channel_;
  OzoneDisplayChannelHost* host_;
  EventFactoryWayland* e_factory_;
  char* spec_;
  static OzoneDisplay* instance_;

  friend class OzoneProcessObserver;
  friend class OzoneDisplayChannelHost;
  friend class OzoneDisplayChannel;
  friend class WaylandScreen;
  DISALLOW_COPY_AND_ASSIGN(OzoneDisplay);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_OZONE_DISPLAY_H_
