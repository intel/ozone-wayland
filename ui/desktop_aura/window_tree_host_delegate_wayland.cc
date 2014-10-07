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
    : current_focus_window_(0),
      handle_event_(true),
      stop_propogation_(false),
      open_windows_(NULL),
      current_dispatcher_(NULL),
      current_capture_(NULL),
      current_active_window_(NULL) {
  if (ui::PlatformEventSource::GetInstance())
    ui::PlatformEventSource::GetInstance()->AddPlatformEventDispatcher(this);
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
    ui::PlatformEventSource* event_source =
        ui::PlatformEventSource::GetInstance();
    if (event_source)
      event_source->RemovePlatformEventDispatcher(this);
    ui::EventFactoryOzoneWayland::GetInstance()->SetWindowChangeObserver(NULL);

    current_active_window_ = NULL;
    current_capture_ = NULL;
    current_dispatcher_ = NULL;
    current_focus_window_ = 0;
    return;
  }

  if (!current_active_window_ ||
      GetWindowHandle(current_active_window_->GetHandle()) !=
          window->GetHandle()) {
     return;
  }

  // Set first top level window in the list of open windows as dispatcher.
  // This is just a guess of the window which would eventually be focussed.
  // We should set the correct root window as dispatcher in OnWindowFocused.
  SetActiveWindow(open_windows().front());
  current_active_window_->GetDelegate()->OnActivationChanged(true);
}

void WindowTreeHostDelegateWayland::SetActiveWindow(
    ui::OzoneWaylandWindow* window) {
  current_active_window_ = window;
  if (!current_active_window_)
    return;

  current_dispatcher_ =
      DesktopWindowTreeHostWayland::GetHostForAcceleratedWidget(
          current_active_window_->GetHandle());
}

void WindowTreeHostDelegateWayland::DeActivateWindow(
    ui::OzoneWaylandWindow* window) {
  if (current_active_window_ != window)
    return;

  current_active_window_ = NULL;
  if (GetWindowHandle(current_dispatcher_->GetAcceleratedWidget()) ==
      current_active_window_->GetHandle()) {
    current_dispatcher_ = NULL;
  }
}

ui::OzoneWaylandWindow* WindowTreeHostDelegateWayland::GetActiveWindow() const {
  return current_active_window_;
}

unsigned
WindowTreeHostDelegateWayland::GetActiveWindowHandle() const {
  return current_active_window_ ? current_active_window_->GetHandle() : 0;
}

void WindowTreeHostDelegateWayland::SetCapture(unsigned handle) {
  if (current_capture_)
    current_capture_->OnCaptureReleased();

  DesktopWindowTreeHostWayland* dispatcher =
      DesktopWindowTreeHostWayland::GetHostForAcceleratedWidget(handle);

  current_capture_ = dispatcher;
  stop_propogation_ = current_capture_ ? true : false;
  current_dispatcher_ = current_capture_;
  if (!current_dispatcher_) {
    current_dispatcher_ =
        DesktopWindowTreeHostWayland::GetHostForAcceleratedWidget(
            current_active_window_->GetHandle());
  }
}

////////////////////////////////////////////////////////////////////////////////
// WindowTreeHostDelegateWayland, Private implementation:
void WindowTreeHostDelegateWayland::DispatchMouseEvent(
         ui::MouseEvent* event) {
  if (handle_event_)
    SendEventToProcessor(event);
  else if (event->type() == ui::ET_MOUSE_PRESSED)
    SetCapture(0);

  // Stop event propogation as this window is acting as event grabber. All
  // event's we create are "cancelable". If in future we use events that are not
  // cancelable, then a check for cancelable events needs to be added here.
  if (stop_propogation_)
    event->StopPropagation();
}

unsigned
WindowTreeHostDelegateWayland::GetWindowHandle(gfx::AcceleratedWidget widget) {
  return static_cast<unsigned>(widget);
}

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

ui::EventProcessor* WindowTreeHostDelegateWayland::GetEventProcessor() {
  return current_dispatcher_->dispatcher();
}

////////////////////////////////////////////////////////////////////////////////
// WindowTreeHostDelegateWayland, ui::PlatformEventDispatcher implementation:
bool WindowTreeHostDelegateWayland::CanDispatchEvent(
    const ui::PlatformEvent& ne) {
  DCHECK(ne);
  return true;
}

uint32_t WindowTreeHostDelegateWayland::DispatchEvent(
    const ui::PlatformEvent& ne) {
  ui::EventType type = ui::EventTypeFromNative(ne);
  DCHECK(current_dispatcher_);

  switch (type) {
    case ui::ET_TOUCH_MOVED:
    case ui::ET_TOUCH_PRESSED:
    case ui::ET_TOUCH_CANCELLED:
    case ui::ET_TOUCH_RELEASED: {
      ui::TouchEvent* touchev = static_cast<ui::TouchEvent*>(ne);
      SendEventToProcessor(touchev);
      break;
    }
    case ui::ET_KEY_PRESSED: {
      ui::KeyEvent* keydown_event = static_cast<ui::KeyEvent*>(ne);
      SendEventToProcessor(keydown_event);
      break;
    }
    case ui::ET_KEY_RELEASED: {
      ui::KeyEvent* keyup_event = static_cast<ui::KeyEvent*>(ne);
      SendEventToProcessor(keyup_event);
      break;
    }
    case ui::ET_MOUSEWHEEL: {
      ui::MouseWheelEvent* wheelev = static_cast<ui::MouseWheelEvent*>(ne);
      DispatchMouseEvent(wheelev);
      break;
    }
    case ui::ET_MOUSE_MOVED:
    case ui::ET_MOUSE_DRAGGED:
    case ui::ET_MOUSE_PRESSED:
    case ui::ET_MOUSE_RELEASED:
    case ui::ET_MOUSE_ENTERED:
    case ui::ET_MOUSE_EXITED: {
      ui::MouseEvent* mouseev = static_cast<ui::MouseEvent*>(ne);
      DispatchMouseEvent(mouseev);
      break;
    }
    case ui::ET_SCROLL_FLING_START:
    case ui::ET_SCROLL_FLING_CANCEL:
    case ui::ET_SCROLL: {
      ui::ScrollEvent* scrollev = static_cast<ui::ScrollEvent*>(ne);
      SendEventToProcessor(scrollev);
      break;
    }
    case ui::ET_UMA_DATA:
      break;
    case ui::ET_UNKNOWN:
      break;
    default:
      NOTIMPLEMENTED() << "WindowTreeHostDelegateWayland: unknown event type.";
  }
  return ui::POST_DISPATCH_STOP_PROPAGATION;
}

////////////////////////////////////////////////////////////////////////////////
// DesktopWindowTreeHostWayland, WindowChangeObserver implementation:
void WindowTreeHostDelegateWayland::OnWindowFocused(unsigned handle) {
  current_focus_window_ = handle;
  // Don't dispatch events in case a window has installed itself as capture
  // window but doesn't have the focus.
  handle_event_ = current_capture_ ? current_focus_window_ ==
          GetWindowHandle(current_capture_->GetAcceleratedWidget()) : true;
  if (current_active_window_) {
    if (current_active_window_->GetHandle() == handle)
      return;

  // A new window should not steal focus in case the current window has a open
  // popup.
    if (current_capture_ && (GetWindowHandle(current_capture_->
        GetAcceleratedWidget()) != current_active_window_->GetHandle())) {
      return;
    }
  }

  ui::OzoneWaylandWindow* window = GetWindow(handle);
  if (!window)
    return;

  if (current_active_window_)
    current_active_window_->GetDelegate()->OnActivationChanged(false);

  SetActiveWindow(GetWindow(handle));
  current_active_window_->GetDelegate()->OnActivationChanged(true);
}

void WindowTreeHostDelegateWayland::OnWindowEnter(unsigned handle) {
  OnWindowFocused(handle);
}

void WindowTreeHostDelegateWayland::OnWindowLeave(unsigned handle) {
}

void WindowTreeHostDelegateWayland::OnWindowClose(unsigned handle) {
  // we specially treat grabbed windows in this function, thus the need for
  // current_capture_ always be a valid pointer.
  if (!handle || !current_capture_)
    return;
  if (GetWindowHandle(current_capture_->GetAcceleratedWidget()) != handle)
    return;

  ui::OzoneWaylandWindow* window = GetWindow(handle);
  DCHECK(window);
  window->GetDelegate()->OnLostCapture();
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
  DesktopWindowTreeHostWayland* window =
      DesktopWindowTreeHostWayland::GetHostForAcceleratedWidget(handle);
  DCHECK(window);
  window->HandleWindowUnminimized();
}

std::list<ui::OzoneWaylandWindow*>&
WindowTreeHostDelegateWayland::open_windows() {
  if (!open_windows_)
    open_windows_ = new std::list<ui::OzoneWaylandWindow*>();

  return *open_windows_;
}

}  // namespace views
