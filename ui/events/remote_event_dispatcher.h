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

  void ChannelEstablished(IPC::Sender* sender);

  virtual void MotionNotify(float x, float y) override;
  virtual void ButtonNotify(unsigned handle,
                            ui::EventType type,
                            ui::EventFlags flags,
                            float x,
                            float y) override;
  virtual void AxisNotify(float x,
                          float y,
                          int xoffset,
                          int yoffset) override;
  virtual void PointerEnter(unsigned handle, float x, float y) override;
  virtual void PointerLeave(unsigned handle, float x, float y) override;
  virtual void KeyNotify(ui::EventType type,
                         unsigned code) override;
  virtual void VirtualKeyNotify(ui::EventType type,
                                uint32_t key,
                                uint32_t modifiers) override;
  virtual void KeyModifiers(uint32_t mods_depressed,
                            uint32_t mods_latched,
                            uint32_t mods_locked,
                            uint32_t group) override;
  virtual void TouchNotify(ui::EventType type,
                           float x,
                           float y,
                           int32_t touch_id,
                           uint32_t time_stamp) override;

  virtual void OutputSizeChanged(unsigned width, unsigned height) override;
  virtual void WindowResized(unsigned handle,
                             unsigned width,
                             unsigned height) override;
  virtual void WindowUnminimized(unsigned windowhandle) override;
  virtual void WindowDeActivated(unsigned windowhandle) override;
  virtual void WindowActivated(unsigned windowhandle) override;
  virtual void CloseWidget(unsigned handle) override;

  virtual void Commit(unsigned handle, const std::string& text) override;
  virtual void PreeditChanged(unsigned handle,
                              const std::string& text,
                              const std::string& commit) override;
  virtual void PreeditEnd() override;
  virtual void PreeditStart() override;
  virtual void InitializeXKB(base::SharedMemoryHandle fd,
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
