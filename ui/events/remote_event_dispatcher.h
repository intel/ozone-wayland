// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_EVENTS_REMOTE_EVENT_DISPATCHER_H_
#define OZONE_UI_EVENTS_REMOTE_EVENT_DISPATCHER_H_

#include <string>

#include "ipc/ipc_sender.h"
#include "ozone/ui/events/event_converter_ozone_wayland.h"

namespace ui {

// RemoteEventDispatcher sends native events from GPU to Browser process over
// IPC. In Multi-process case, callbacks from Wayland are received in GPU
// process side. All callbacks related to input need to be handled in Browser
// process and hence the events are sent to it over IPC.
class RemoteEventDispatcher : public ui::EventConverterOzoneWayland {
 public:
  RemoteEventDispatcher();
  virtual ~RemoteEventDispatcher();

  void ChannelEstablished(IPC::Sender* sender) override;

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

  void OutputSizeChanged(unsigned width, unsigned height) override;
  void WindowResized(unsigned handle,
                     unsigned width,
                     unsigned height) override;
  void WindowUnminimized(unsigned windowhandle) override;
  void WindowDeActivated(unsigned windowhandle) override;
  void WindowActivated(unsigned windowhandle) override;
  void CloseWidget(unsigned handle) override;

  void Commit(unsigned handle, const std::string& text) override;
  void PreeditChanged(unsigned handle,
                      const std::string& text,
                      const std::string& commit) override;
  void PreeditEnd() override;
  void PreeditStart() override;
  void InitializeXKB(base::SharedMemoryHandle fd,
                     uint32_t size) override;

 private:
  void Dispatch(IPC::Message* message);
  static void Send(RemoteEventDispatcher* dispatcher,
                   IPC::Message* message);
  IPC::Sender* sender_;
  DISALLOW_COPY_AND_ASSIGN(RemoteEventDispatcher);
};

}  // namespace ui

#endif  // OZONE_UI_EVENTS_REMOTE_EVENT_DISPATCHER_H_
