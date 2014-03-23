// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_DISPLAY_H_
#define OZONE_WAYLAND_DISPLAY_H_

#if !defined(__STDC_FORMAT_MACROS)
#define __STDC_FORMAT_MACROS
#endif

#include <wayland-client.h>
#include <list>
#include <map>

#include "base/basictypes.h"
#include "ozone/ui/events/window_state_change_handler.h"
#include "ozone/wayland/input/text-client-protocol.h"

struct xdg_shell;
namespace ozonewayland {

class WaylandDisplayPollThread;
class WaylandInputDevice;
class WaylandScreen;
class WaylandWindow;
struct wl_egl_window;

typedef std::map<unsigned, WaylandWindow*> WindowMap;

// WaylandDisplay is a wrapper around wl_display. Once we get a valid
// wl_display, the Wayland server will send different events to register
// the Wayland compositor, shell, screens, input devices, ...
class WaylandDisplay : public ui::WindowStateChangeHandler {
 public:
  enum RegistrationType {
    RegisterAsNeeded,  // Handles all the required registrations.
    RegisterOutputOnly  // Only screen registration.
  };

  explicit WaylandDisplay(RegistrationType type);
  virtual ~WaylandDisplay();

  // Ownership is not passed to the caller.
  static WaylandDisplay* GetInstance() { return instance_; }
  static void LookAheadOutputGeometry();
  // Returns a pointer to wl_display.
  wl_display* display() const { return display_; }

  wl_registry* registry() const { return registry_; }

  WaylandInputDevice* PrimaryInput() const { return primary_input_; }

  // Returns a list of the registered screens.
  const std::list<WaylandScreen*>& GetScreenList() const;
  WaylandScreen* PrimaryScreen() const { return primary_screen_ ; }

  wl_shell* shell() const { return shell_; }
  xdg_shell* xdgshell() const { return xdg_shell_; }

  wl_shm* shm() const { return shm_; }
  wl_compositor* GetCompositor() const { return compositor_; }
  struct wl_text_input_manager* GetTextInputManager() const;

  int GetDisplayFd() const { return wl_display_get_fd(display_); }
  unsigned GetSerial() const { return serial_; }
  void SetSerial(unsigned serial) { serial_ = serial; }
  // Returns WaylandWindow associated with w. The ownership is not transferred
  // to the caller.
  WaylandWindow* GetWindow(unsigned window_handle) const;

  // Flush Display.
  void FlushDisplay();
  // Does a round trip to Wayland server. This call blocks the current thread
  // until all pending request are processed by the server.
  void SyncDisplay();
  // Realizes an AcceleratedWidget so that the returned AcceleratedWidget
  // can be used to to create a GLSurface. Ownership is not passed to the
  // caller.
  wl_egl_window* RealizeAcceleratedWidget(unsigned w);
  // WindowStateChangeHandler implementation:
  virtual void SetWidgetState(unsigned widget,
                              ui::WidgetState state,
                              unsigned width = 0,
                              unsigned height = 0) OVERRIDE;
  virtual void SetWidgetTitle(unsigned w,
                              const base::string16& title) OVERRIDE;
  virtual void SetWidgetAttributes(unsigned widget,
                                   unsigned parent,
                                   unsigned x,
                                   unsigned y,
                                   ui::WidgetType type) OVERRIDE;

 private:
  // Creates a WaylandWindow backed by EGL Window and maps it to w. This can be
  // useful for callers to track a particular surface. By default the type of
  // surface(i.e. toplevel, menu) is none. One needs to explicitly call
  // WaylandWindow::SetShellAttributes to set this. The ownership of
  // WaylandWindow is not passed to the caller.
  WaylandWindow* CreateAcceleratedSurface(unsigned w);

  // Destroys WaylandWindow whose handle is w.
  void DestroyWindow(unsigned w);

  // Starts polling on display fd. This should be used when one needs to
  // continuously read pending events coming from Wayland compositor and
  // dispatch them. The polling is done completely on a separate thread and
  // doesn't block the thread from which this is called.
  void StartProcessingEvents();
  // Stops polling on display fd.
  void StopProcessingEvents();

  void terminate();
  WaylandWindow* GetWidget(unsigned w) const;
  // This handler resolves all server events used in initialization. It also
  // handles input device registration, screen registration.
  static void DisplayHandleGlobal(
      void *data,
      struct wl_registry *registry,
      uint32_t name,
      const char *interface,
      uint32_t version);
  // This handler resolves only screen registration. In general you don't want
  // to use this but the one below.
  static void DisplayHandleOutputOnly(
      void *data,
      struct wl_registry *registry,
      uint32_t name,
      const char *interface,
      uint32_t version);

  // WaylandDisplay manages the memory of all these pointers.
  wl_display* display_;
  wl_registry* registry_;
  wl_compositor* compositor_;
  wl_shell* shell_;
  xdg_shell* xdg_shell_;
  wl_shm* shm_;
  struct wl_text_input_manager* text_input_manager_;
  WaylandScreen* primary_screen_;
  WaylandInputDevice* primary_input_;
  WaylandDisplayPollThread* display_poll_thread_;

  std::list<WaylandScreen*> screen_list_;
  std::list<WaylandInputDevice*> input_list_;
  WindowMap widget_map_;
  unsigned serial_;
  bool processing_events_;
  static WaylandDisplay* instance_;
  DISALLOW_COPY_AND_ASSIGN(WaylandDisplay);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_DISPLAY_H_
