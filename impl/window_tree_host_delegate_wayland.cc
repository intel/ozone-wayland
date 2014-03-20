// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/impl/window_tree_host_delegate_wayland.h"

#include "ozone/impl/desktop_window_tree_host_wayland.h"
#include "ozone/impl/ozone_display.h"
#include "ozone/ui/events/event_converter_ozone_wayland.h"
#include "ui/events/event_utils.h"

using namespace ozonewayland;

namespace views {

WindowTreeHostDelegateWayland::WindowTreeHostDelegateWayland()
    : current_focus_window_(0),
      handle_event_(true),
      stop_propogation_(false),
      current_dispatcher_(NULL),
      current_capture_(NULL),
      current_active_window_(NULL),
      open_windows_(NULL),
      aura_windows_(NULL) {
  DCHECK(base::MessagePumpOzone::Current());
  base::MessagePumpOzone::Current()->AddDispatcherForRootWindow(this);
  EventConverterOzoneWayland::GetInstance()->SetWindowChangeObserver(this);
}

WindowTreeHostDelegateWayland::~WindowTreeHostDelegateWayland() {
}

void WindowTreeHostDelegateWayland::OnRootWindowCreated(unsigned handle) {
  open_windows().push_back(handle);

  if (aura_windows_) {
    aura_windows_->clear();
    delete aura_windows_;
    aura_windows_ = NULL;
  }
}

void WindowTreeHostDelegateWayland::OnRootWindowClosed(unsigned handle) {
  open_windows().remove(handle);
  if (open_windows().empty()) {
    delete open_windows_;
    open_windows_ = NULL;
    SetActiveWindow(NULL);

    DCHECK(base::MessagePumpOzone::Current());
    base::MessagePumpOzone::Current()->RemoveDispatcherForRootWindow(this);
    EventConverterOzoneWayland::GetInstance()->SetWindowChangeObserver(NULL);
  }

  if (aura_windows_) {
    aura_windows_->clear();
    delete aura_windows_;
    aura_windows_ = NULL;
  }

  if (!current_active_window_ ||
      GetWindowHandle(current_active_window_->window_) != handle ||
      !open_windows_) {
     return;
  }

  DCHECK(!current_active_window_->window_parent_);
  // Set first top level window in the list of open windows as dispatcher.
  // This is just a guess of the window which would eventually be focussed.
  // We should set the correct root window as dispatcher in OnWindowFocused.
  const std::list<unsigned>& windows = open_windows();
  DesktopWindowTreeHostWayland* rootWindow =
      DesktopWindowTreeHostWayland::GetHostForAcceleratedWidget(
          windows.front());
  SetActiveWindow(rootWindow);
  rootWindow->HandleNativeWidgetActivationChanged(true);
}

void WindowTreeHostDelegateWayland::SetActiveWindow(
    DesktopWindowTreeHostWayland* dispatcher) {
  current_active_window_ = dispatcher;
  current_dispatcher_ = current_active_window_;
  if (!current_active_window_)
    return;

  // Make sure the stacking order is correct. The activated window should be
  // first one in list of open windows.
  std::list<unsigned>& windows = open_windows();
  DCHECK(windows.size());
  unsigned window_handle = current_active_window_->window_;
  if (windows.front() != window_handle) {
    windows.remove(window_handle);
    windows.insert(windows.begin(), window_handle);
  }

  current_active_window_->Activate();
}

DesktopWindowTreeHostWayland*
WindowTreeHostDelegateWayland::GetActiveWindow() const {
  return current_active_window_;
}

void WindowTreeHostDelegateWayland::SetCapture(
    DesktopWindowTreeHostWayland* dispatcher) {
  if (current_capture_)
    current_capture_->OnCaptureReleased();

  current_capture_ = dispatcher;
  stop_propogation_ = current_capture_ ? true : false;
  current_dispatcher_ = current_capture_;
  if (!current_dispatcher_)
    current_dispatcher_ = current_active_window_;
}

DesktopWindowTreeHostWayland*
WindowTreeHostDelegateWayland::GetCurrentCapture() const {
  return current_capture_;
}

const std::vector<aura::Window*>&
WindowTreeHostDelegateWayland::GetAllOpenWindows() {
  if (!aura_windows_) {
    const std::list<unsigned>& windows = open_windows();
    DCHECK(windows.size());
    aura_windows_ = new std::vector<aura::Window*>(windows.size());
    std::transform(
        windows.begin(), windows.end(), aura_windows_->begin(),
            DesktopWindowTreeHostWayland::GetContentWindowForAcceleratedWidget);
  }

  return *aura_windows_;
}

////////////////////////////////////////////////////////////////////////////////
// WindowTreeHostDelegateWayland, Private implementation:
void WindowTreeHostDelegateWayland::DispatchMouseEvent(
         ui::MouseEvent* event) {
  if (handle_event_)
    SendEventToProcessor(event);
  else if (event->type() == ui::ET_MOUSE_PRESSED)
    SetCapture(NULL);

  // Stop event propogation as this window is acting as event grabber. All
  // event's we create are "cancelable". If in future we use events that are not
  // cancelable, then a check for cancelable events needs to be added here.
  if (stop_propogation_)
    event->StopPropagation();
}

std::list<unsigned>&
WindowTreeHostDelegateWayland::open_windows() {
  if (!open_windows_)
    open_windows_ = new std::list<unsigned>();

  return *open_windows_;
}

unsigned
WindowTreeHostDelegateWayland::GetWindowHandle(gfx::AcceleratedWidget widget) {
  return static_cast<unsigned>(widget);
}

ui::EventProcessor* WindowTreeHostDelegateWayland::GetEventProcessor() {
  return current_dispatcher_->delegate_->GetEventProcessor();
}

////////////////////////////////////////////////////////////////////////////////
// WindowTreeHostDelegateWayland, MessagePumpDispatcher implementation:
uint32_t WindowTreeHostDelegateWayland::Dispatch(const base::NativeEvent& ne) {
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
  return POST_DISPATCH_NONE;
}

////////////////////////////////////////////////////////////////////////////////
// DesktopWindowTreeHostWayland, WindowChangeObserver implementation:
void WindowTreeHostDelegateWayland::OnWindowFocused(unsigned handle) {
  current_focus_window_ = handle;
  // Don't dispatch events in case a window has installed itself as capture
  // window but doesn't have the focus.
  handle_event_ = current_capture_ ? current_focus_window_ ==
          GetWindowHandle(current_capture_->GetAcceleratedWidget()) : true;
  if (GetWindowHandle(current_active_window_->window_) == handle)
    return;

  // A new window should not steal focus in case the current window has a open
  // popup.
  if (current_capture_ && current_capture_ != current_active_window_)
    return;

  DesktopWindowTreeHostWayland* window = NULL;
  if (handle)
    window = DesktopWindowTreeHostWayland::GetHostForAcceleratedWidget(handle);

  if (!window || window->window_parent_)
    return;

  current_active_window_->HandleNativeWidgetActivationChanged(false);

  SetActiveWindow(window);
  window->HandleNativeWidgetActivationChanged(true);
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
  if (GetWindowHandle(current_capture_->window_) != handle)
    return;
  DesktopWindowTreeHostWayland* window = NULL;
  window = DesktopWindowTreeHostWayland::GetHostForAcceleratedWidget(handle);
  window->OnCaptureReleased();
  window->Close();
}

void WindowTreeHostDelegateWayland::OnWindowResized(unsigned handle,
                                                    unsigned width,
                                                    unsigned height) {
  DesktopWindowTreeHostWayland* window =
      DesktopWindowTreeHostWayland::GetHostForAcceleratedWidget(handle);
  DCHECK(window);
  window->HandleWindowResize(width, height);
}

}  // namespace views
