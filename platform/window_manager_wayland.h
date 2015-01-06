// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_IMPL_PLATFORM_WINDOW_MANAGER_OZONE_H_
#define OZONE_IMPL_PLATFORM_WINDOW_MANAGER_OZONE_H_

#include <list>
#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/message_loop/message_loop.h"
#include "ozone/ui/events/window_change_observer.h"
#include "ui/events/event.h"
#include "ui/events/event_source.h"
#include "ui/events/platform/platform_event_dispatcher.h"
#include "ui/gfx/native_widget_types.h"

namespace ui {
class OzoneWaylandWindow;
}

namespace ui {

// A static class used by OzoneWaylandWindow for basic window management.
class WindowManagerWayland
    : public ui::WindowChangeObserver {
 public:
  WindowManagerWayland();
  ~WindowManagerWayland() override;

  void OnRootWindowCreated(ui::OzoneWaylandWindow* window);
  void OnRootWindowClosed(ui::OzoneWaylandWindow* window);

  ui::OzoneWaylandWindow* GetWindow(unsigned handle);
  bool HasWindowsOpen() const;

 private:
  // Window Change Observer.
  void OnWindowFocused(unsigned handle) override;
  void OnWindowEnter(unsigned handle) override;
  void OnWindowLeave(unsigned handle) override;
  void OnWindowClose(unsigned handle) override;
  void OnWindowResized(unsigned windowhandle,
                       unsigned width,
                       unsigned height) override;
  void OnWindowUnminimized(unsigned windowhandle) override;
  void OnWindowDeActivated(unsigned windowhandle) override;
  void OnWindowActivated(unsigned windowhandle) override;

  // Dispatches a mouse event.
  std::list<ui::OzoneWaylandWindow*>& open_windows();

  // List of all open aura::Window.
  std::list<ui::OzoneWaylandWindow*>* open_windows_;
  DISALLOW_COPY_AND_ASSIGN(WindowManagerWayland);
};

}  // namespace ui

#endif  // OZONE_IMPL_PLATFORM_WINDOW_MANAGER_OZONE_H_
