// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_EVENTS_EVENT_CONVERTER_IN_PROCESS_H_
#define OZONE_UI_EVENTS_EVENT_CONVERTER_IN_PROCESS_H_

#include <string>

#include "base/memory/scoped_ptr.h"
#include "base/timer/timer.h"
#include "ozone/ui/events/event_converter_ozone_wayland.h"
#include "ui/events/event.h"
#include "ui/events/ozone/evdev/event_modifiers_evdev.h"
#include "ui/events/ozone/evdev/keyboard_evdev.h"
#include "ui/events/platform/platform_event_source.h"
#include "ui/ozone/public/gpu_platform_support_host.h"

namespace ui {

class DrmGpuPlatformSupportHost;
class KeyboardEvdev;
class EventConverterInProcess : public ui::EventConverterOzoneWayland,
                                public ui::PlatformEventSource,
                                public GpuPlatformSupportHost {
 public:
  explicit EventConverterInProcess(DrmGpuPlatformSupportHost* proxy);
  ~EventConverterInProcess() override;
  // GpuPlatformSupportHost
  void OnChannelEstablished(
      int host_id,
      scoped_refptr<base::SingleThreadTaskRunner> send_runner,
      const base::Callback<void(IPC::Message*)>& send_callback) override;
  void OnChannelDestroyed(int host_id) override;
  bool OnMessageReceived(const IPC::Message&) override;

  // EventConverterOzoneWayland
  void MotionNotify(float x, float y) override;
  void ButtonNotify(unsigned handle,
                    ui::EventType type,
                    ui::EventFlags flags,
                    float x,
                    float y) override;
  void AxisNotify(float x,
                  float y,
                  int xoffset,
                  int yoffset) override;
  void PointerEnter(unsigned handle, float x, float y) override;
  void PointerLeave(unsigned handle, float x, float y) override;
  void KeyNotify(ui::EventType type,
                 unsigned code) override;
  void VirtualKeyNotify(ui::EventType type,
                        uint32_t key) override;
  void TouchNotify(ui::EventType type,
                   float x,
                   float y,
                   int32_t touch_id,
                   uint32_t time_stamp) override;
  void CloseWidget(unsigned handle) override;

  void OutputSizeChanged(unsigned width, unsigned height) override;
  void WindowResized(unsigned windowhandle,
                     unsigned width,
                     unsigned height) override;
  void WindowUnminimized(unsigned windowhandle) override;
  void WindowDeActivated(unsigned windowhandle) override;
  void WindowActivated(unsigned windowhandle) override;

  void Commit(unsigned handle, const std::string& text) override;
  void PreeditChanged(unsigned handle,
                      const std::string& text,
                      const std::string& commit) override;
  void PreeditEnd() override;
  void PreeditStart() override;
  void InitializeXKB(base::SharedMemoryHandle fd, uint32_t size) override;

 private:
  // PlatformEventSource:
  void OnDispatcherListChanged() override;

  // Dispatch event via PlatformEventSource.
  void DispatchUiEventTask(scoped_ptr<Event> event);
  // Post a task to dispatch an event.
  void PostUiEvent(Event* event);

  void NotifyMotion(float x,
                    float y);
  void NotifyButtonPress(unsigned handle,
                         ui::EventType type,
                         ui::EventFlags flags,
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
  void NotifyTouchEvent(ui::EventType type,
                        float x,
                        float y,
                        int32_t touch_id,
                        uint32_t time_stamp);
  void NotifyCloseWidget(unsigned handle);
  void NotifyOutputSizeChanged(unsigned width,
                               unsigned height);
  void NotifyWindowResized(unsigned handle,
                           unsigned width,
                           unsigned height);
  void NotifyWindowUnminimized(unsigned handle);
  void NotifyWindowDeActivated(unsigned handle);
  void NotifyWindowActivated(unsigned handle);
  void NotifyCommit(unsigned handle,
                    const std::string& text);
  void NotifyPreeditChanged(unsigned handle,
                            const std::string& text,
                            const std::string& commit);
  void NotifyPreeditEnd();
  void NotifyPreeditStart();

  // Modifier key state (shift, ctrl, etc).
  EventModifiersEvdev modifiers_;
  // Keyboard state.
  KeyboardEvdev keyboard_;
  DrmGpuPlatformSupportHost* proxy_;
  // Callback for dispatching events.
  EventDispatchCallback callback_;
  // Support weak pointers for attach & detach callbacks.
  base::WeakPtrFactory<EventConverterInProcess> weak_ptr_factory_;
  DISALLOW_COPY_AND_ASSIGN(EventConverterInProcess);
};

}  // namespace ui

#endif  // OZONE_UI_EVENTS_EVENT_CONVERTER_IN_PROCESS_H_
