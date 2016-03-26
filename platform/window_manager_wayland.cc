// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/platform/window_manager_wayland.h"

#include <sys/mman.h>
#include <string>

#include "base/bind.h"
#include "base/thread_task_runner_handle.h"
#include "ozone/platform/desktop_platform_screen_delegate.h"
#include "ozone/platform/messages.h"
#include "ozone/platform/ozone_gpu_platform_support_host.h"
#include "ozone/platform/ozone_wayland_window.h"
#include "ozone/wayland/ozone_wayland_screen.h"
#include "ui/aura/window.h"
#include "ui/events/event_utils.h"
#include "ui/events/ozone/layout/keyboard_layout_engine_manager.h"
#include "ui/events/platform/platform_event_source.h"
#include "ui/platform_window/platform_window_delegate.h"

namespace ui {

WindowManagerWayland::WindowManagerWayland(OzoneGpuPlatformSupportHost* proxy)
    : open_windows_(NULL),
      active_window_(NULL),
      proxy_(proxy),
      keyboard_(&modifiers_,
                KeyboardLayoutEngineManager::GetKeyboardLayoutEngine(),
                base::Bind(&WindowManagerWayland::PostUiEvent,
                           base::Unretained(this))),
      platform_screen_(NULL),
      weak_ptr_factory_(this) {
  proxy_->RegisterHandler(this);
}

WindowManagerWayland::~WindowManagerWayland() {
}

void WindowManagerWayland::OnRootWindowCreated(
    OzoneWaylandWindow* window) {
  open_windows().push_back(window);
}

void WindowManagerWayland::OnRootWindowClosed(
    OzoneWaylandWindow* window) {
  open_windows().remove(window);
  if (open_windows().empty()) {
    delete open_windows_;
    open_windows_ = NULL;
    return;
  }

  if (active_window_ == window)
     active_window_ = NULL;

  if (event_grabber_ == window->GetHandle())
    event_grabber_ = gfx::kNullAcceleratedWidget;

  if (current_capture_ == window->GetHandle()) {
     OzoneWaylandWindow* window = GetWindow(current_capture_);
     window->GetDelegate()->OnLostCapture();
    current_capture_ = gfx::kNullAcceleratedWidget;
  }

  // Set first top level window in the list of open windows as dispatcher.
  // This is just a guess of the window which would eventually be focussed.
  // We should set the correct root window as dispatcher in OnWindowFocused.
  OnActivationChanged(open_windows().front()->GetHandle(), true);
}

void WindowManagerWayland::Restore(OzoneWaylandWindow* window) {
  active_window_ = window;
  event_grabber_  = window->GetHandle();
}

void WindowManagerWayland::OnPlatformScreenCreated(
      ozonewayland::OzoneWaylandScreen* screen) {
  DCHECK(!platform_screen_);
  platform_screen_ = screen;
}

PlatformCursor WindowManagerWayland::GetPlatformCursor() {
  return platform_cursor_;
}

void WindowManagerWayland::SetPlatformCursor(PlatformCursor cursor) {
  platform_cursor_ = cursor;
}

bool WindowManagerWayland::HasWindowsOpen() const {
  return open_windows_ ? !open_windows_->empty() : false;
}

void WindowManagerWayland::GrabEvents(gfx::AcceleratedWidget widget) {
  if (current_capture_ == widget)
    return;

  if (current_capture_) {
    OzoneWaylandWindow* window = GetWindow(current_capture_);
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

OzoneWaylandWindow*
WindowManagerWayland::GetWindow(unsigned handle) {
  OzoneWaylandWindow* window = NULL;
  const std::list<OzoneWaylandWindow*>& windows = open_windows();
  std::list<OzoneWaylandWindow*>::const_iterator it;
  for (it = windows.begin(); it != windows.end(); ++it) {
    if ((*it)->GetHandle() == handle) {
      window = *it;
      break;
    }
  }

  return window;
}

////////////////////////////////////////////////////////////////////////////////
// WindowManagerWayland, Private implementation:
void WindowManagerWayland::OnActivationChanged(unsigned windowhandle,
                                               bool active) {
  OzoneWaylandWindow* window = GetWindow(windowhandle);
  if (!window) {
    LOG(ERROR) << "Invalid window handle " << windowhandle;
    return;
  }

  if (active) {
    if (active_window_ && active_window_ == window)
        return;

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

std::list<OzoneWaylandWindow*>&
WindowManagerWayland::open_windows() {
  if (!open_windows_)
    open_windows_ = new std::list<OzoneWaylandWindow*>();

  return *open_windows_;
}

void WindowManagerWayland::OnWindowFocused(unsigned handle) {
  OnActivationChanged(handle, true);
}

void WindowManagerWayland::OnWindowEnter(unsigned handle) {
  OnWindowFocused(handle);
}

void WindowManagerWayland::OnWindowLeave(unsigned handle) {
}

void WindowManagerWayland::OnWindowClose(unsigned handle) {
  OzoneWaylandWindow* window = GetWindow(handle);
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
  OzoneWaylandWindow* window = GetWindow(handle);
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
  OzoneWaylandWindow* window = GetWindow(handle);
  if (!window) {
    LOG(ERROR) << "Received invalid window handle " << handle
               << " from GPU process";
    return;
  }

  window->GetDelegate()->OnWindowStateChanged(PLATFORM_WINDOW_STATE_MAXIMIZED);
}

void WindowManagerWayland::OnWindowDeActivated(unsigned windowhandle) {
  OnActivationChanged(windowhandle, false);
}

void WindowManagerWayland::OnWindowActivated(unsigned windowhandle) {
  OnWindowFocused(windowhandle);
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
  IPC_MESSAGE_HANDLER(WaylandInput_MotionNotify, MotionNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_ButtonNotify, ButtonNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_TouchNotify, TouchNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_AxisNotify, AxisNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_PointerEnter, PointerEnter)
  IPC_MESSAGE_HANDLER(WaylandInput_PointerLeave, PointerLeave)
  IPC_MESSAGE_HANDLER(WaylandInput_KeyNotify, KeyNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_VirtualKeyNotify, VirtualKeyNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_OutputSize, OutputSizeChanged)
  IPC_MESSAGE_HANDLER(WaylandInput_InitializeXKB, InitializeXKB)
  IPC_MESSAGE_HANDLER(WaylandInput_DragEnter, DragEnter)
  IPC_MESSAGE_HANDLER(WaylandInput_DragData, DragData)
  IPC_MESSAGE_HANDLER(WaylandInput_DragLeave, DragLeave)
  IPC_MESSAGE_HANDLER(WaylandInput_DragMotion, DragMotion)
  IPC_MESSAGE_HANDLER(WaylandInput_DragDrop, DragDrop)
  IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void WindowManagerWayland::MotionNotify(float x, float y) {
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::Bind(&WindowManagerWayland::NotifyMotion,
          weak_ptr_factory_.GetWeakPtr(), x, y));
}

void WindowManagerWayland::ButtonNotify(unsigned handle,
                                        EventType type,
                                        EventFlags flags,
                                        float x,
                                        float y) {
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::Bind(&WindowManagerWayland::NotifyButtonPress,
          weak_ptr_factory_.GetWeakPtr(), handle, type, flags, x, y));
}

void WindowManagerWayland::AxisNotify(float x,
                                      float y,
                                      int xoffset,
                                      int yoffset) {
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::Bind(&WindowManagerWayland::NotifyAxis,
          weak_ptr_factory_.GetWeakPtr(), x, y, xoffset, yoffset));
}

void WindowManagerWayland::PointerEnter(unsigned handle,
                                        float x,
                                        float y) {
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::Bind(&WindowManagerWayland::NotifyPointerEnter,
          weak_ptr_factory_.GetWeakPtr(), handle, x, y));
}

void WindowManagerWayland::PointerLeave(unsigned handle,
                                        float x,
                                        float y) {
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::Bind(&WindowManagerWayland::NotifyPointerLeave,
          weak_ptr_factory_.GetWeakPtr(), handle, x, y));
}

void WindowManagerWayland::KeyNotify(EventType type,
                                     unsigned code,
                                     int device_id) {
  VirtualKeyNotify(type, code, device_id);
}

void WindowManagerWayland::VirtualKeyNotify(EventType type,
                                            uint32_t key,
                                            int device_id) {
  keyboard_.OnKeyChange(key,
                        type != ET_KEY_RELEASED,
                        false,
                        EventTimeForNow(),
                        device_id);
}

void WindowManagerWayland::TouchNotify(EventType type,
                                       float x,
                                       float y,
                                       int32_t touch_id,
                                       uint32_t time_stamp) {
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::Bind(&WindowManagerWayland::NotifyTouchEvent,
          weak_ptr_factory_.GetWeakPtr(), type, x, y, touch_id, time_stamp));
}

void WindowManagerWayland::CloseWidget(unsigned handle) {
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::Bind(&WindowManagerWayland::OnWindowClose,
          weak_ptr_factory_.GetWeakPtr(), handle));
}

void WindowManagerWayland::OutputSizeChanged(unsigned width,
                                             unsigned height) {
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::Bind(&WindowManagerWayland::NotifyOutputSizeChanged,
          weak_ptr_factory_.GetWeakPtr(), width, height));
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

void WindowManagerWayland::DragEnter(
    unsigned windowhandle,
    float x,
    float y,
    const std::vector<std::string>& mime_types,
    uint32_t serial) {
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::Bind(&WindowManagerWayland::NotifyDragEnter,
          weak_ptr_factory_.GetWeakPtr(),
          windowhandle, x, y, mime_types, serial));
}

void WindowManagerWayland::DragData(unsigned windowhandle,
                                    base::FileDescriptor pipefd) {
  // TODO(mcatanzaro): pipefd will be leaked if the WindowManagerWayland is
  // destroyed before NotifyDragData is called.
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::Bind(&WindowManagerWayland::NotifyDragData,
          weak_ptr_factory_.GetWeakPtr(), windowhandle, pipefd));
}

void WindowManagerWayland::DragLeave(unsigned windowhandle) {
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::Bind(&WindowManagerWayland::NotifyDragLeave,
          weak_ptr_factory_.GetWeakPtr(), windowhandle));
}

void WindowManagerWayland::DragMotion(unsigned windowhandle,
                                      float x,
                                      float y,
                                      uint32_t time) {
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::Bind(&WindowManagerWayland::NotifyDragMotion,
          weak_ptr_factory_.GetWeakPtr(), windowhandle, x, y, time));
}

void WindowManagerWayland::DragDrop(unsigned windowhandle) {
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::Bind(&WindowManagerWayland::NotifyDragDrop,
          weak_ptr_factory_.GetWeakPtr(), windowhandle));
}

void WindowManagerWayland::InitializeXKB(base::SharedMemoryHandle fd,
                                         uint32_t size) {
  char* map_str =
      reinterpret_cast<char*>(mmap(NULL,
                                   size,
                                   PROT_READ,
                                   MAP_SHARED,
                                   fd.fd,
                                   0));
  if (map_str == MAP_FAILED)
    return;

  KeyboardLayoutEngineManager::GetKeyboardLayoutEngine()->
      SetCurrentLayoutByName(map_str);
  munmap(map_str, size);
  close(fd.fd);
}

////////////////////////////////////////////////////////////////////////////////
// PlatformEventSource implementation:
void WindowManagerWayland::PostUiEvent(Event* event) {
  DispatchEvent(event);
}

void WindowManagerWayland::DispatchUiEventTask(scoped_ptr<Event> event) {
  DispatchEvent(event.get());
}

void WindowManagerWayland::OnDispatcherListChanged() {
}

////////////////////////////////////////////////////////////////////////////////
void WindowManagerWayland::NotifyMotion(float x,
                                        float y) {
  gfx::Point position(x, y);
  MouseEvent mouseev(ET_MOUSE_MOVED,
                         position,
                         position,
                         EventTimeForNow(),
                         0,
                         0);
  DispatchEvent(&mouseev);
}

void WindowManagerWayland::NotifyButtonPress(unsigned handle,
                                             EventType type,
                                             EventFlags flags,
                                             float x,
                                             float y) {
  gfx::Point position(x, y);
  MouseEvent mouseev(type,
                         position,
                         position,
                         EventTimeForNow(),
                         flags,
                         flags);

  DispatchEvent(&mouseev);

  if (type == ET_MOUSE_RELEASED)
    OnWindowFocused(handle);
}

void WindowManagerWayland::NotifyAxis(float x,
                                         float y,
                                         int xoffset,
                                         int yoffset) {
  gfx::Point position(x, y);
  MouseEvent mouseev(ET_MOUSEWHEEL,
                         position,
                         position,
                         EventTimeForNow(),
                         0,
                         0);

  MouseWheelEvent wheelev(mouseev, xoffset, yoffset);

  DispatchEvent(&wheelev);
}

void WindowManagerWayland::NotifyPointerEnter(unsigned handle,
                                                 float x,
                                                 float y) {
  OnWindowEnter(handle);

  gfx::Point position(x, y);
  MouseEvent mouseev(ET_MOUSE_ENTERED,
                         position,
                         position,
                         EventTimeForNow(),
                         0,
                         0);

  DispatchEvent(&mouseev);
}

void WindowManagerWayland::NotifyPointerLeave(unsigned handle,
                                              float x,
                                              float y) {
  OnWindowLeave(handle);

  gfx::Point position(x, y);
  MouseEvent mouseev(ET_MOUSE_EXITED,
                         position,
                         position,
                         EventTimeForNow(),
                         0,
                         0);

  DispatchEvent(&mouseev);
}

void WindowManagerWayland::NotifyTouchEvent(EventType type,
                                            float x,
                                            float y,
                                            int32_t touch_id,
                                            uint32_t time_stamp) {
  gfx::Point position(x, y);
  base::TimeDelta time_delta = base::TimeDelta::FromMilliseconds(time_stamp);
  TouchEvent touchev(type, position, touch_id, time_delta);
  DispatchEvent(&touchev);
}

void WindowManagerWayland::NotifyOutputSizeChanged(unsigned width,
                                                   unsigned height) {
  if (platform_screen_)
    platform_screen_->GetDelegate()->OnOutputSizeChanged(width, height);
}

void WindowManagerWayland::NotifyDragEnter(
    unsigned windowhandle,
    float x,
    float y,
    const std::vector<std::string>& mime_types,
    uint32_t serial) {
  ui::OzoneWaylandWindow* window = GetWindow(windowhandle);
  if (!window) {
    LOG(ERROR) << "Received invalid window handle " << windowhandle
               << " from GPU process";
    return;
  }
  window->GetDelegate()->OnDragEnter(windowhandle, x, y, mime_types, serial);
}

void WindowManagerWayland::NotifyDragData(unsigned windowhandle,
                                          base::FileDescriptor pipefd) {
  ui::OzoneWaylandWindow* window = GetWindow(windowhandle);
  if (!window) {
    LOG(ERROR) << "Received invalid window handle " << windowhandle
               << " from GPU process";
    close(pipefd.fd);
    return;
  }
  window->GetDelegate()->OnDragDataReceived(pipefd.fd);
}

void WindowManagerWayland::NotifyDragLeave(unsigned windowhandle) {
  ui::OzoneWaylandWindow* window = GetWindow(windowhandle);
  if (!window) {
    LOG(ERROR) << "Received invalid window handle " << windowhandle
               << " from GPU process";
    return;
  }
  window->GetDelegate()->OnDragLeave();
}

void WindowManagerWayland::NotifyDragMotion(unsigned windowhandle,
                                            float x,
                                            float y,
                                            uint32_t time) {
  ui::OzoneWaylandWindow* window = GetWindow(windowhandle);
  if (!window) {
    LOG(ERROR) << "Received invalid window handle " << windowhandle
               << " from GPU process";
    return;
  }
  window->GetDelegate()->OnDragMotion(x, y, time);
}

void WindowManagerWayland::NotifyDragDrop(unsigned windowhandle) {
  ui::OzoneWaylandWindow* window = GetWindow(windowhandle);
  if (!window) {
    LOG(ERROR) << "Received invalid window handle " << windowhandle
               << " from GPU process";
    return;
  }
  window->GetDelegate()->OnDragDrop();
}

}  // namespace ui
