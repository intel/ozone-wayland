// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_EVENTS_REMOTE_EVENT_DISPATCHER_H_
#define OZONE_UI_EVENTS_REMOTE_EVENT_DISPATCHER_H_

#include "content/browser/gpu/gpu_process_host.h"
#include "ozone/ui/events/event_converter_ozone_wayland.h"

namespace ozonewayland {

// RemoteEventDispatcher sends native events from GPU to Browser process over
// IPC. In Multi-process case, callbacks from Wayland are received in GPU
// process side. All callbacks related to input need to be handled in Browser
// process and hence the events are sent to it over IPC.
class RemoteEventDispatcher : public EventConverterOzoneWayland {
  public:
  RemoteEventDispatcher();
  virtual ~RemoteEventDispatcher();

  virtual void MotionNotify(float x, float y) OVERRIDE;
  virtual void ButtonNotify(unsigned handle,
                            ui::EventType type,
                            ui::EventFlags flags,
                            float x,
                            float y) OVERRIDE;
  virtual void AxisNotify(float x,
                          float y,
                          int xoffset,
                          int yoffset) OVERRIDE;
  virtual void PointerEnter(unsigned handle, float x, float y) OVERRIDE;
  virtual void PointerLeave(unsigned handle, float x, float y) OVERRIDE;
  virtual void KeyNotify(ui::EventType type,
                         unsigned code,
                         unsigned modifiers) OVERRIDE;

  virtual void OutputSizeChanged(unsigned width, unsigned height) OVERRIDE;
  virtual void WindowResized(unsigned handle,
                             unsigned width,
                             unsigned height) OVERRIDE;
  virtual void CloseWidget(unsigned handle) OVERRIDE;

 private:
  static void SendMotionNotify(float x, float y);
  static void SendButtonNotify(unsigned handle,
                               ui::EventType type,
                               ui::EventFlags flags,
                               float x,
                               float y);
  static void SendAxisNotify(float x, float y, int xoffset, int yoffset);
  static void SendPointerEnter(unsigned handle, float x, float y);
  static void SendPointerLeave(unsigned handle, float x, float y);
  static void SendKeyNotify(ui::EventType type,
                            unsigned code,
                            unsigned modifiers);
  static void SendOutputSizeChanged(unsigned width, unsigned height);
  static void SendWindowResized(unsigned handle,
                                unsigned width,
                                unsigned height);
  static void SendCloseWidget(unsigned handle);
  static void Send(IPC::Message* message);
  DISALLOW_COPY_AND_ASSIGN(RemoteEventDispatcher);
};

}  // namespace ozonewayland

#endif  // OZONE_UI_EVENTS_REMOTE_EVENT_DISPATCHER_H_
