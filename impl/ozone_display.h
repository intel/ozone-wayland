// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_IMPL_OZONE_DISPLAY_H_
#define OZONE_IMPL_OZONE_DISPLAY_H_

#include "base/compiler_specific.h"
#include "base/message_loop/message_loop.h"
#include "ozone/platform/ozone_export_wayland.h"
#include "ui/gfx/ozone/surface_factory_ozone.h"

namespace ozonewayland {

class DesktopScreenWayland;
class EventConverterOzoneWayland;
class OzoneProcessObserver;
class OzoneDisplayChannel;
class OzoneDisplayChannelHost;
class WaylandDisplay;
class WaylandWindow;
class WaylandScreen;
class WindowChangeObserver;

class OZONE_WAYLAND_EXPORT OzoneDisplay
    : public gfx::SurfaceFactoryOzone,
      public base::MessageLoop::DestructionObserver {
 public:
  enum {
    Create = 1,  // Create a new Widget
    Show = 2,  // Widget is visible.
    Hide = 3,  // Widget is hidden.
    FullScreen = 4,  // Widget is in fullscreen mode.
    Maximized = 5,  // Widget is maximized,
    Minimized = 6,  // Widget is minimized.
    Restore = 7,  // Restore Widget.
    Active = 8,  // Widget is Activated.
    InActive = 9,  // Widget is DeActivated.
    Resize = 10,  // Widget is Resized.
    Destroyed = 11  // Widget is Destroyed.
  };

  typedef unsigned WidgetState;

  enum {
    Window = 1,  // A decorated Window.
    WindowFrameLess = 2,  // An undecorated Window.
    Popup = 3 // An undecorated Window, with transient positioning relative to
              // its parent and in which the input pointer is implicit grabbed
              // (i.e. Wayland install the grab) by the Window.
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

  // MessageLoop::DestructionObserver overrides.
  virtual void WillDestroyCurrentMessageLoop() OVERRIDE;

  const DesktopScreenWayland* GetPrimaryScreen() const;

  void SetWidgetState(gfx::AcceleratedWidget w,
                      WidgetState state,
                      unsigned width = 0,
                      unsigned height = 0);
  void OnWidgetStateChanged(gfx::AcceleratedWidget w,
                            WidgetState state,
                            unsigned width,
                            unsigned height);
  void SetWidgetTitle(gfx::AcceleratedWidget w, const base::string16& title);
  void OnWidgetTitleChanged(gfx::AcceleratedWidget w,
                            const base::string16& title);
  void SetWidgetAttributes(gfx::AcceleratedWidget widget,
                           gfx::AcceleratedWidget parent,
                           unsigned x,
                           unsigned y,
                           WidgetType type);
  void OnWidgetAttributesChanged(gfx::AcceleratedWidget widget,
                                 gfx::AcceleratedWidget parent,
                                 unsigned x,
                                 unsigned y,
                                 WidgetType type);
  void OnOutputSizeChanged(unsigned width, unsigned height);
  void SetWindowChangeObserver(WindowChangeObserver* observer);

  // EstablishChannel is called by OzoneProcessObserver (i.e. on BrowserProcess
  // side) when GPU Process is connected. This can be used to establish an IPC
  // channel between DisplayChannelHost and DisplayChannel.
  void EstablishChannel();
  // OnChannelEstablished is called by DisplayChannel (i.e. on GPUProcess side)
  // when the IPC Channel connection between itself and DisplayChannelHost is
  // established successfully.
  void OnChannelEstablished();
  // OnChannelHostDestroyed is called by DisplayChannelHost (i.e. on
  // BrowserProcess side) when the IPC Channel connection between itself and
  // DisplayChannel is closed.
  void OnChannelHostDestroyed();

 private:
  enum State {
    UnInitialized = 0x00,
    Initialized = 0x01,
    ChannelConnected = 0x02
  };

  typedef unsigned CurrentState;

  void OnOutputSizeChanged(WaylandScreen* screen, int width, int height);
  void CreateWidget(unsigned w);
  WaylandWindow* GetWidget(gfx::AcceleratedWidget w);

  void Terminate();
  void InitializeDispatcher(int fd = 0);
  void LookAheadOutputGeometry();

  static void DelayedInitialization(OzoneDisplay* display);

  CurrentState state_;
  gfx::SurfaceFactoryOzone::HardwareState initialized_state_;
  unsigned last_realized_widget_handle_;
  const int kMaxDisplaySize_;

  DesktopScreenWayland* desktop_screen_;
  WaylandDisplay* display_;
  OzoneProcessObserver* child_process_observer_;
  OzoneDisplayChannel* channel_;
  OzoneDisplayChannelHost* host_;
  EventConverterOzoneWayland* event_converter_;
  char* spec_;
  static OzoneDisplay* instance_;

  friend class WaylandScreen;
  DISALLOW_COPY_AND_ASSIGN(OzoneDisplay);
};

}  // namespace ozonewayland

#endif  // OZONE_IMPL_OZONE_DISPLAY_H_
