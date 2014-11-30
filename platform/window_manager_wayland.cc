// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/platform/window_manager_wayland.h"

#include <string>

#include "ozone/platform/ozone_wayland_window.h"
#include "ozone/ui/events/event_factory_ozone_wayland.h"
#include "ui/aura/window.h"
#include "ui/events/event_utils.h"
#include "ui/events/platform/platform_event_source.h"
#include "ui/platform_window/platform_window_delegate.h"

namespace ui {

WindowManagerWayland::WindowManagerWayland()
    : open_windows_(NULL) {
  ui::EventFactoryOzoneWayland::GetInstance()->SetWindowChangeObserver(this);
}

WindowManagerWayland::~WindowManagerWayland() {
}

void WindowManagerWayland::OnRootWindowCreated(
    ui::OzoneWaylandWindow* window) {
  open_windows().push_back(window);
}

void WindowManagerWayland::OnRootWindowClosed(
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

bool WindowManagerWayland::HasWindowsOpen() const {
  return open_windows_ ? !open_windows_->empty() : false;
}

////////////////////////////////////////////////////////////////////////////////
// WindowManagerWayland, Private implementation:
ui::OzoneWaylandWindow*
WindowManagerWayland::GetWindow(unsigned handle) {
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
// WindowManagerWayland, WindowChangeObserver implementation:
void WindowManagerWayland::OnWindowFocused(unsigned handle) {
  DCHECK(handle);
  ui::OzoneWaylandWindow* window = GetWindow(handle);
  if (!window)
    return;

  window->GetDelegate()->OnActivationChanged(true);
}

void WindowManagerWayland::OnWindowEnter(unsigned handle) {
  OnWindowFocused(handle);
}

void WindowManagerWayland::OnWindowLeave(unsigned handle) {
}

void WindowManagerWayland::OnWindowClose(unsigned handle) {
  DCHECK(handle);
  ui::OzoneWaylandWindow* window = GetWindow(handle);
  DCHECK(window);
  window->GetDelegate()->OnCloseRequest();
}

void WindowManagerWayland::OnWindowResized(unsigned handle,
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

void WindowManagerWayland::OnWindowUnminimized(unsigned handle) {
  ui::OzoneWaylandWindow* window = GetWindow(handle);
  DCHECK(window);
  window->GetDelegate()->OnWindowStateChanged(
      ui::PLATFORM_WINDOW_STATE_MAXIMIZED);
}

void WindowManagerWayland::OnWindowDeActivated(unsigned windowhandle) {
  DCHECK(windowhandle);
  ui::OzoneWaylandWindow* window = GetWindow(windowhandle);
  DCHECK(window);
  window->GetDelegate()->OnActivationChanged(false);
}

void WindowManagerWayland::OnWindowActivated(unsigned windowhandle) {
  DCHECK(windowhandle);
  OnWindowFocused(windowhandle);
}

std::list<ui::OzoneWaylandWindow*>&
WindowManagerWayland::open_windows() {
  if (!open_windows_)
    open_windows_ = new std::list<ui::OzoneWaylandWindow*>();

  return *open_windows_;
}

}  // namespace ui
