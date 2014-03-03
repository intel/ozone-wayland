// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_IMPL_DESKTOP_AURA_WINDOW_TREE_HOST_DELEGATE_WAYLAND_H_
#define OZONE_IMPL_DESKTOP_AURA_WINDOW_TREE_HOST_DELEGATE_WAYLAND_H_

#include <list>
#include <vector>

#include "base/basictypes.h"
#include "base/message_loop/message_loop.h"
#include "ozone/ui/events/window_change_observer.h"
#include "ui/events/event.h"
#include "ui/events/event_source.h"
#include "ui/gfx/native_widget_types.h"

namespace ozonewayland {

class DesktopWindowTreeHostWayland;

// A static class used by DesktopWindowTreeHostWayland to dispatch native events
// and basic window management.
class WindowTreeHostDelegateWayland
    : public base::MessagePumpDispatcher,
      public ui::EventSource,
      public ui::WindowChangeObserver {
 public:
  WindowTreeHostDelegateWayland();
  virtual ~WindowTreeHostDelegateWayland();

  void OnRootWindowCreated(unsigned handle);
  void OnRootWindowClosed(unsigned handle);

  void SetActiveWindow(DesktopWindowTreeHostWayland* dispatcher);
  DesktopWindowTreeHostWayland* GetActiveWindow() const;

  void SetCapture(DesktopWindowTreeHostWayland* dispatcher);
  DesktopWindowTreeHostWayland* GetCurrentCapture() const;

  const std::vector<aura::Window*>& GetAllOpenWindows();

  // Overridden frm ui::EventSource
  virtual ui::EventProcessor* GetEventProcessor() OVERRIDE;
 private:
  // Overridden from Dispatcher:
  virtual uint32_t Dispatch(const base::NativeEvent& event) OVERRIDE;
  // Window Change Observer.
  virtual void OnWindowFocused(unsigned handle) OVERRIDE;
  virtual void OnWindowEnter(unsigned handle) OVERRIDE;
  virtual void OnWindowLeave(unsigned handle) OVERRIDE;
  virtual void OnWindowClose(unsigned handle) OVERRIDE;
  virtual void OnWindowResized(unsigned windowhandle,
                               unsigned width,
                               unsigned height) OVERRIDE;

  // Dispatches a mouse event.
  void DispatchMouseEvent(ui::MouseEvent* event);
  std::list<gfx::AcceleratedWidget>& open_windows();

  unsigned current_focus_window_;
  bool handle_event_ :1;
  bool stop_propogation_ :1;

  // Current dispatcher.
  DesktopWindowTreeHostWayland* current_dispatcher_;
  // The current root window host that has capture. We need to track this so we
  // can notify widgets when they have lost capture, which controls a bunch of
  // things in views like hiding menus.
  DesktopWindowTreeHostWayland* current_capture_;
  DesktopWindowTreeHostWayland* current_active_window_;
  // List of all open windows.
  std::list<gfx::AcceleratedWidget>* open_windows_;
  // List of all open aura::Window.
  std::vector<aura::Window*>* aura_windows_;
  DISALLOW_COPY_AND_ASSIGN(WindowTreeHostDelegateWayland);
};

}  // namespace ozonewayland

#endif  // OZONE_IMPL_DESKTOP_AURA_WINDOW_TREE_HOST_DELEGATE_WAYLAND_H_
