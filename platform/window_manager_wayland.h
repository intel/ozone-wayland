// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_IMPL_PLATFORM_WINDOW_MANAGER_OZONE_H_
#define OZONE_IMPL_PLATFORM_WINDOW_MANAGER_OZONE_H_

#include <list>
#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/memory/shared_memory.h"
#include "base/memory/weak_ptr.h"
#include "ui/base/cursor/cursor.h"
#include "ui/events/event.h"
#include "ui/events/event_source.h"
#include "ui/events/ozone/evdev/event_modifiers_evdev.h"
#include "ui/events/ozone/evdev/keyboard_evdev.h"
#include "ui/events/platform/platform_event_dispatcher.h"
#include "ui/events/platform/platform_event_source.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/ozone/public/gpu_platform_support_host.h"

namespace ozonewayland {
class OzoneWaylandScreen;
}

namespace ui {

class OzoneGpuPlatformSupportHost;
class OzoneWaylandWindow;

// A static class used by OzoneWaylandWindow for basic window management.
class WindowManagerWayland
    : public PlatformEventSource,
      public GpuPlatformSupportHost {
 public:
  explicit WindowManagerWayland(OzoneGpuPlatformSupportHost* proxy);
  ~WindowManagerWayland() override;

  void OnRootWindowCreated(OzoneWaylandWindow* window);
  void OnRootWindowClosed(OzoneWaylandWindow* window);
  void Restore(OzoneWaylandWindow* window);

  void OnPlatformScreenCreated(ozonewayland::OzoneWaylandScreen* screen);

  PlatformCursor GetPlatformCursor();
  void SetPlatformCursor(PlatformCursor cursor);

  OzoneWaylandWindow* GetWindow(unsigned handle);
  bool HasWindowsOpen() const;

  OzoneWaylandWindow* GetActiveWindow() const { return active_window_; }

  // Tries to set a given widget as the recipient for events. It will
  // fail if there is already another widget as recipient.
  void GrabEvents(gfx::AcceleratedWidget widget);

  // Unsets a given widget as the recipient for events.
  void UngrabEvents(gfx::AcceleratedWidget widget);

  // Gets the current widget recipient of mouse events.
  gfx::AcceleratedWidget event_grabber() const { return event_grabber_; }

 private:
  void OnActivationChanged(unsigned windowhandle, bool active);
  std::list<OzoneWaylandWindow*>& open_windows();
  void OnWindowFocused(unsigned handle);
  void OnWindowEnter(unsigned handle);
  void OnWindowLeave(unsigned handle);
  void OnWindowClose(unsigned handle);
  void OnWindowResized(unsigned windowhandle,
                       unsigned width,
                       unsigned height);
  void OnWindowUnminimized(unsigned windowhandle);
  void OnWindowDeActivated(unsigned windowhandle);
  void OnWindowActivated(unsigned windowhandle);
  // GpuPlatformSupportHost
  void OnChannelEstablished(
      int host_id,
      scoped_refptr<base::SingleThreadTaskRunner> send_runner,
      const base::Callback<void(IPC::Message*)>& send_callback) override;
  void OnChannelDestroyed(int host_id) override;
  bool OnMessageReceived(const IPC::Message&) override;
  void MotionNotify(float x, float y);
  void ButtonNotify(unsigned handle,
                    EventType type,
                    EventFlags flags,
                    float x,
                    float y);
  void AxisNotify(float x,
                  float y,
                  int xoffset,
                  int yoffset);
  void PointerEnter(unsigned handle, float x, float y);
  void PointerLeave(unsigned handle, float x, float y);
  void KeyNotify(EventType type, unsigned code, int device_id);
  void VirtualKeyNotify(EventType type,
                        uint32_t key,
                        int device_id);
  void TouchNotify(EventType type,
                   float x,
                   float y,
                   int32_t touch_id,
                   uint32_t time_stamp);
  void CloseWidget(unsigned handle);

  void OutputSizeChanged(unsigned width, unsigned height);
  void WindowResized(unsigned windowhandle,
                     unsigned width,
                     unsigned height);
  void WindowUnminimized(unsigned windowhandle);
  void WindowDeActivated(unsigned windowhandle);
  void WindowActivated(unsigned windowhandle);

  void DragEnter(unsigned windowhandle,
                 float x,
                 float y,
                 const std::vector<std::string>& mime_types,
                 uint32_t serial);
  void DragData(unsigned windowhandle, base::FileDescriptor pipefd);
  void DragLeave(unsigned windowhandle);
  void DragMotion(unsigned windowhandle, float x, float y, uint32_t time);
  void DragDrop(unsigned windowhandle);

  void InitializeXKB(base::SharedMemoryHandle fd, uint32_t size);
  // PlatformEventSource:
  void OnDispatcherListChanged() override;

  // Dispatch event via PlatformEventSource.
  void DispatchUiEventTask(scoped_ptr<Event> event);
  // Post a task to dispatch an event.
  void PostUiEvent(Event* event);

  void NotifyMotion(float x,
                    float y);
  void NotifyButtonPress(unsigned handle,
                         EventType type,
                         EventFlags flags,
                         float x,
                         float y);
  void NotifyAxis(float x,
                  float y,
                  int xoffset,
                  int yoffset);
  void NotifyPointerEnter(unsigned handle,
                          float x,
                          float y);
  void NotifyPointerLeave(unsigned handle,
                          float x,
                          float y);
  void NotifyTouchEvent(EventType type,
                        float x,
                        float y,
                        int32_t touch_id,
                        uint32_t time_stamp);
  void NotifyOutputSizeChanged(unsigned width,
                               unsigned height);

  void NotifyDragEnter(unsigned windowhandle,
                       float x,
                       float y,
                       const std::vector<std::string>& mime_types,
                       uint32_t serial);
  void NotifyDragData(unsigned windowhandle, base::FileDescriptor pipefd);
  void NotifyDragLeave(unsigned windowhandle);
  void NotifyDragMotion(unsigned windowhandle, float x, float y, uint32_t time);
  void NotifyDragDrop(unsigned windowhandle);

  // List of all open aura::Window.
  std::list<OzoneWaylandWindow*>* open_windows_;
  gfx::AcceleratedWidget event_grabber_ = gfx::kNullAcceleratedWidget;
  OzoneWaylandWindow* active_window_;
  gfx::AcceleratedWidget current_capture_ = gfx::kNullAcceleratedWidget;
  OzoneGpuPlatformSupportHost* proxy_;
  // Modifier key state (shift, ctrl, etc).
  EventModifiersEvdev modifiers_;
  // Keyboard state.
  KeyboardEvdev keyboard_;
  ozonewayland::OzoneWaylandScreen* platform_screen_;
  PlatformCursor platform_cursor_;
  // Support weak pointers for attach & detach callbacks.
  base::WeakPtrFactory<WindowManagerWayland> weak_ptr_factory_;
  DISALLOW_COPY_AND_ASSIGN(WindowManagerWayland);
};

}  // namespace ui

#endif  // OZONE_IMPL_PLATFORM_WINDOW_MANAGER_OZONE_H_
