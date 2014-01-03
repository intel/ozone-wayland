// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_IMPL_ROOT_WINDOW_HOST_DELEGATE_WAYLAND_H_
#define OZONE_IMPL_ROOT_WINDOW_HOST_DELEGATE_WAYLAND_H_

#include <list>
#include <vector>

#include "base/basictypes.h"
#include "base/message_loop/message_loop.h"
#include "ozone/wayland/window_change_observer.h"
#include "ui/events/event.h"
#include "ui/gfx/native_widget_types.h"

namespace ozonewayland {

class DesktopRootWindowHostWayland;

// A static class used by DesktopRootWindowHostWayland to dispatch native events
// and basic window management.
class RootWindowHostDelegateWayland
    : public base::MessageLoop::Dispatcher,
      public WindowChangeObserver {
 public:
  RootWindowHostDelegateWayland();
  virtual ~RootWindowHostDelegateWayland();

  void OnRootWindowCreated(unsigned handle);
  void OnRootWindowClosed(unsigned handle);

  void SetActiveWindow(DesktopRootWindowHostWayland* dispatcher);
  DesktopRootWindowHostWayland* GetActiveWindow() const;

  void SetCapture(DesktopRootWindowHostWayland* dispatcher);
  DesktopRootWindowHostWayland* GetCurrentCapture() const;

  std::vector<aura::Window*>& GetAllOpenWindows();
 private:
  // Overridden from Dispatcher:
  virtual bool Dispatch(const base::NativeEvent& event) OVERRIDE;
  // Window Change Observer.
  virtual void OnWindowFocused(unsigned windowhandle) OVERRIDE;
  virtual void OnWindowEnter(unsigned windowhandle) OVERRIDE;
  virtual void OnWindowLeave(unsigned windowhandle) OVERRIDE;

  // Dispatches a mouse event, taking mouse capture into account. If a
  // different host has capture, we translate the event to its coordinate space
  // and dispatch it to that host instead.
  void DispatchMouseEvent(ui::MouseEvent* event);
  std::list<gfx::AcceleratedWidget>& open_windows();

  unsigned current_focus_window_;
  bool handle_event_ :1;
  bool stop_propogation_ :1;

  // Current dispatcher.
  DesktopRootWindowHostWayland* current_dispatcher_;
  // The current root window host that has capture. We need to track this so we
  // can notify widgets when they have lost capture, which controls a bunch of
  // things in views like hiding menus.
  DesktopRootWindowHostWayland* current_capture_;
  DesktopRootWindowHostWayland* current_active_window_;
  // List of all open windows.
  std::list<gfx::AcceleratedWidget>* open_windows_;
  // List of all open aura::Window.
  std::vector<aura::Window*>* aura_windows_;
  DISALLOW_COPY_AND_ASSIGN(RootWindowHostDelegateWayland);
};

}  // namespace ozonewayland

#endif  // OZONE_IMPL_ROOT_WINDOW_HOST_DELEGATE_WAYLAND_H_
