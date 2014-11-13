// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/desktop_aura/window_tree_host_delegate_wayland.h"

#include <string>

#include "ozone/platform/ozone_wayland_window.h"
#include "ozone/ui/desktop_aura/desktop_window_tree_host_wayland.h"
#include "ozone/ui/events/event_factory_ozone_wayland.h"
#include "ui/aura/window.h"
#include "ui/events/event_utils.h"
#include "ui/events/platform/platform_event_source.h"
#include "ui/platform_window/platform_window_delegate.h"

namespace views {

WindowTreeHostDelegateWayland::WindowTreeHostDelegateWayland()
    : open_windows_(NULL) {
  ui::EventFactoryOzoneWayland::GetInstance()->SetWindowChangeObserver(this);
}

WindowTreeHostDelegateWayland::~WindowTreeHostDelegateWayland() {
}

void WindowTreeHostDelegateWayland::OnRootWindowCreated(
    ui::OzoneWaylandWindow* window) {
  open_windows().push_back(window);
}

void WindowTreeHostDelegateWayland::OnRootWindowClosed(
    ui::OzoneWaylandWindow* window) {
  open_windows().remove(window);
  if (open_windows().empty()) {
    delete open_windows_;
    open_windows_ = NULL;
    ui::EventFactoryOzoneWayland::GetInstance()->SetWindowChangeObserver(NULL);
    return;
  }

  // Set first top level window in the list of open windows as dispatcher.
  // This is just a guess of the window which would eventually be focussed.
  // We should set the correct root window as dispatcher in OnWindowFocused.
  ui::OzoneWaylandWindow* new_window = open_windows().front();
  new_window->GetDelegate()->OnActivationChanged(true);
}

 bool WindowTreeHostDelegateWayland::HasWindowsOpen() const {
  return open_windows_ ? !open_windows_->empty() : false;
 }

////////////////////////////////////////////////////////////////////////////////
// WindowTreeHostDelegateWayland, Private implementation:
ui::OzoneWaylandWindow*
WindowTreeHostDelegateWayland::GetWindow(unsigned handle) {
  ui::OzoneWaylandWindow* window = NULL;
  const std::list<ui::OzoneWaylandWindow*>& windows = open_windows();
  std::list<ui::OzoneWaylandWindow*>::const_iterator it;
  for (it = windows.begin(); it != windows.end(); ++it) {
    if ((*it)->GetHandle() == handle) {
      window = *it;
      break;
    }
  }

  return window;
}

////////////////////////////////////////////////////////////////////////////////
// WindowTreeHostDelegateWayland, WindowChangeObserver implementation:
void WindowTreeHostDelegateWayland::OnWindowFocused(unsigned handle) {
  DCHECK(handle);
  ui::OzoneWaylandWindow* window = GetWindow(handle);
  if (!window)
    return;

  window->GetDelegate()->OnActivationChanged(true);
}

void WindowTreeHostDelegateWayland::OnWindowEnter(unsigned handle) {
  OnWindowFocused(handle);
}

void WindowTreeHostDelegateWayland::OnWindowLeave(unsigned handle) {
}

void WindowTreeHostDelegateWayland::OnWindowClose(unsigned handle) {
  DCHECK(handle);
  ui::OzoneWaylandWindow* window = GetWindow(handle);
  DCHECK(window);
  window->GetDelegate()->OnCloseRequest();
}

void WindowTreeHostDelegateWayland::OnWindowResized(unsigned handle,
                                                    unsigned width,
                                                    unsigned height) {
  ui::OzoneWaylandWindow* window = GetWindow(handle);
  DCHECK(window);
  const gfx::Rect& current_bounds = window->GetBounds();
  window->SetBounds(gfx::Rect(current_bounds.x(),
                              current_bounds.y(),
                              width,
                              height));
}

void WindowTreeHostDelegateWayland::OnWindowUnminimized(unsigned handle) {
  ui::OzoneWaylandWindow* window = GetWindow(handle);
  DCHECK(window);
  window->GetDelegate()->OnWindowStateChanged(
      ui::PLATFORM_WINDOW_STATE_MAXIMIZED);
}

void WindowTreeHostDelegateWayland::OnWindowDeActivated(unsigned windowhandle) {
  DCHECK(windowhandle);
  ui::OzoneWaylandWindow* window = GetWindow(windowhandle);
  DCHECK(window);
  window->GetDelegate()->OnActivationChanged(false);
}

void WindowTreeHostDelegateWayland::OnWindowActivated(unsigned windowhandle) {
  DCHECK(windowhandle);
  OnWindowFocused(windowhandle);
}

std::list<ui::OzoneWaylandWindow*>&
WindowTreeHostDelegateWayland::open_windows() {
  if (!open_windows_)
    open_windows_ = new std::list<ui::OzoneWaylandWindow*>();

  return *open_windows_;
}

}  // namespace views
