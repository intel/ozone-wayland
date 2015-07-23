// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/platform/window_manager_wayland.h"

#include <string>

#include "base/bind.h"
#include "base/thread_task_runner_handle.h"
#include "ozone/platform/ozone_gpu_platform_support_host.h"
#include "ozone/platform/ozone_wayland_window.h"
#include "ozone/ui/events/event_factory_ozone_wayland.h"
#include "ozone/ui/public/messages.h"
#include "ui/aura/window.h"
#include "ui/events/event_utils.h"
#include "ui/events/platform/platform_event_source.h"
#include "ui/platform_window/platform_window_delegate.h"

namespace ui {

WindowManagerWayland::WindowManagerWayland(OzoneGpuPlatformSupportHost* proxy)
    : open_windows_(NULL),
      active_window_(NULL),
      proxy_(proxy),
      weak_ptr_factory_(this) {
  ui::EventFactoryOzoneWayland::GetInstance()->SetWindowChangeObserver(this);
  proxy_->RegisterHandler(this);
}

WindowManagerWayland::~WindowManagerWayland() {
  proxy_->UnregisterHandler(this);
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

  if (active_window_ == window)
     active_window_ = NULL;

  if (event_grabber_ == window->GetHandle())
    event_grabber_ = gfx::kNullAcceleratedWidget;

  if (current_capture_ == window->GetHandle()) {
     ui::OzoneWaylandWindow* window = GetWindow(current_capture_);
     window->GetDelegate()->OnLostCapture();
    current_capture_ = gfx::kNullAcceleratedWidget;
  }

  // Set first top level window in the list of open windows as dispatcher.
  // This is just a guess of the window which would eventually be focussed.
  // We should set the correct root window as dispatcher in OnWindowFocused.
  OnActivationChanged(open_windows().front()->GetHandle(), true);
}

void WindowManagerWayland::Restore(ui::OzoneWaylandWindow* window) {
  active_window_ = window;
  event_grabber_  = window->GetHandle();
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
// GpuPlatformSupportHost implementation:
void WindowManagerWayland::OnChannelEstablished(
  int host_id, scoped_refptr<base::SingleThreadTaskRunner> send_runner,
      const base::Callback<void(IPC::Message*)>& send_callback) {
}

void WindowManagerWayland::OnChannelDestroyed(int host_id) {
}

bool WindowManagerWayland::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(WindowManagerWayland, message)
  IPC_MESSAGE_HANDLER(WaylandInput_CloseWidget, CloseWidget)
  IPC_MESSAGE_HANDLER(WaylandWindow_Resized, WindowResized)
  IPC_MESSAGE_HANDLER(WaylandWindow_Activated, WindowActivated)
  IPC_MESSAGE_HANDLER(WaylandWindow_DeActivated, WindowDeActivated)
  IPC_MESSAGE_HANDLER(WaylandWindow_Unminimized, WindowUnminimized)
  IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void WindowManagerWayland::WindowResized(unsigned handle,
                                         unsigned width,
                                         unsigned height) {
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::Bind(&WindowManagerWayland::OnWindowResized,
          weak_ptr_factory_.GetWeakPtr(), handle, width, height));
}

void WindowManagerWayland::WindowUnminimized(unsigned handle) {
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::Bind(&WindowManagerWayland::OnWindowUnminimized,
          weak_ptr_factory_.GetWeakPtr(), handle));
}

void WindowManagerWayland::WindowDeActivated(unsigned windowhandle) {
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::Bind(&WindowManagerWayland::OnWindowDeActivated,
          weak_ptr_factory_.GetWeakPtr(), windowhandle));
}

void WindowManagerWayland::WindowActivated(unsigned windowhandle) {
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::Bind(&WindowManagerWayland::OnWindowActivated,
          weak_ptr_factory_.GetWeakPtr(), windowhandle));
}

void WindowManagerWayland::CloseWidget(unsigned handle) {
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::Bind(&WindowManagerWayland::OnWindowClose,
          weak_ptr_factory_.GetWeakPtr(), handle));
}

////////////////////////////////////////////////////////////////////////////////
// WindowManagerWayland, WindowChangeObserver implementation:
void WindowManagerWayland::OnWindowFocused(unsigned handle) {
  OnActivationChanged(handle, true);
}

void WindowManagerWayland::OnWindowEnter(unsigned handle) {
  OnWindowFocused(handle);
}

void WindowManagerWayland::OnWindowLeave(unsigned handle) {
}

void WindowManagerWayland::OnWindowClose(unsigned handle) {
  ui::OzoneWaylandWindow* window = GetWindow(handle);
  if (!window) {
    LOG(ERROR) << "Received invalid window handle " << handle
               << " from GPU process";
    return;
  }

  window->GetDelegate()->OnCloseRequest();
}

void WindowManagerWayland::OnWindowResized(unsigned handle,
                                           unsigned width,
                                           unsigned height) {
  ui::OzoneWaylandWindow* window = GetWindow(handle);
  if (!window) {
    LOG(ERROR) << "Received invalid window handle " << handle
               << " from GPU process";
    return;
  }

  const gfx::Rect& current_bounds = window->GetBounds();
  window->SetBounds(gfx::Rect(current_bounds.x(),
                              current_bounds.y(),
                              width,
                              height));
}

void WindowManagerWayland::OnWindowUnminimized(unsigned handle) {
  ui::OzoneWaylandWindow* window = GetWindow(handle);
  if (!window) {
    LOG(ERROR) << "Received invalid window handle " << handle
               << " from GPU process";
    return;
  }

  window->GetDelegate()->OnWindowStateChanged(
      ui::PLATFORM_WINDOW_STATE_MAXIMIZED);
}

void WindowManagerWayland::OnWindowDeActivated(unsigned windowhandle) {
  OnActivationChanged(windowhandle, false);
}

void WindowManagerWayland::OnWindowActivated(unsigned windowhandle) {
  OnWindowFocused(windowhandle);
}

std::list<ui::OzoneWaylandWindow*>&
WindowManagerWayland::open_windows() {
  if (!open_windows_)
    open_windows_ = new std::list<ui::OzoneWaylandWindow*>();

  return *open_windows_;
}

void WindowManagerWayland::GrabEvents(gfx::AcceleratedWidget widget) {
  if (current_capture_ == widget)
    return;

  if (current_capture_) {
    ui::OzoneWaylandWindow* window = GetWindow(current_capture_);
    window->GetDelegate()->OnLostCapture();
  }

  current_capture_ = widget;
  event_grabber_ = widget;
}

void WindowManagerWayland::UngrabEvents(gfx::AcceleratedWidget widget) {
  if (current_capture_ != widget)
    return;

  current_capture_ = gfx::kNullAcceleratedWidget;
  event_grabber_ = active_window_ ? active_window_->GetHandle() : 0;
}

void WindowManagerWayland::OnActivationChanged(unsigned windowhandle,
                                               bool active) {
  ui::OzoneWaylandWindow* window = GetWindow(windowhandle);
  if (!window) {
    LOG(ERROR) << "Invalid window handle " << windowhandle;
    return;
  }

  if (active) {
    if (current_capture_) {
      event_grabber_ = windowhandle;
      return;
    }

    if (active_window_)
      active_window_->GetDelegate()->OnActivationChanged(false);

    event_grabber_ = windowhandle;
    active_window_ = window;
    active_window_->GetDelegate()->OnActivationChanged(active);
  } else if (active_window_ == window) {
      active_window_->GetDelegate()->OnActivationChanged(active);
      if (event_grabber_ == active_window_->GetHandle())
         event_grabber_ = gfx::kNullAcceleratedWidget;

      active_window_ = NULL;
  }
}

}  // namespace ui
