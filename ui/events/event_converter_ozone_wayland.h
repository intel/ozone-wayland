// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_EVENTS_EVENT_CONVERTER_OZONE_WAYLAND_H_
#define OZONE_UI_EVENTS_EVENT_CONVERTER_OZONE_WAYLAND_H_

#include <string>

#include "base/memory/shared_memory.h"
#include "base/message_loop/message_loop.h"
#include "ipc/ipc_sender.h"
#include "ozone/platform/ozone_export_wayland.h"
#include "ui/events/event_constants.h"

namespace ui {

class WindowChangeObserver;
class OutputChangeObserver;

// In OzoneWayland, Chrome relies on Wayland protocol to recieve callback of
// any input/surface events. This class is responsible for the following:
// 1) Convert native input events to |ui::Event| and dispatching to aura.
// 2) Convert native events related to Window Activation and dispatch it to
//    WindowChangeObserver
class OZONE_WAYLAND_EXPORT EventConverterOzoneWayland {
 public:
  EventConverterOzoneWayland() { }
  virtual ~EventConverterOzoneWayland() { }

  virtual void ChannelEstablished(IPC::Sender* sender) { }
  virtual void MotionNotify(float x, float y) = 0;
  virtual void ButtonNotify(unsigned handle,
                            EventType type,
                            EventFlags flags,
                            float x,
                            float y) = 0;
  virtual void AxisNotify(float x, float y, int xoffset, int yoffset) = 0;
  virtual void PointerEnter(unsigned handle, float x, float y) = 0;
  virtual void PointerLeave(unsigned handle, float x, float y) = 0;
  virtual void KeyNotify(ui::EventType type,
                         unsigned code,
                         int device_id) = 0;
  virtual void VirtualKeyNotify(ui::EventType type,
                                uint32_t key,
                                int device_id) = 0;
  virtual void TouchNotify(ui::EventType type,
                           float x,
                           float y,
                           int32_t touch_id,
                           uint32_t time_stamp) = 0;

  virtual void OutputSizeChanged(unsigned width, unsigned height) = 0;
  virtual void WindowResized(unsigned windowhandle,
                             unsigned width,
                             unsigned height) = 0;
  virtual void WindowUnminimized(unsigned windowhandle) = 0;
  virtual void WindowDeActivated(unsigned windowhandle) = 0;
  virtual void WindowActivated(unsigned windowhandle) = 0;
  virtual void CloseWidget(unsigned handle) = 0;
  virtual void Commit(unsigned handle, const std::string& text) = 0;
  virtual void PreeditChanged(unsigned handle, const std::string& text,
                              const std::string& commit) = 0;
  virtual void PreeditEnd() = 0;
  virtual void PreeditStart() = 0;
  virtual void InitializeXKB(base::SharedMemoryHandle fd,
                             uint32_t size) = 0;
};

}  // namespace ui

#endif  // OZONE_UI_EVENTS_EVENT_CONVERTER_OZONE_WAYLAND_H_
