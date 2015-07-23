// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_PLATFORM_GPU_EVENT_DISPATCHER_H_
#define OZONE_PLATFORM_GPU_EVENT_DISPATCHER_H_

#include <queue>
#include <string>

#include "base/memory/shared_memory.h"
#include "base/message_loop/message_loop.h"
#include "ipc/ipc_sender.h"
#include "ozone/platform/ozone_export_wayland.h"
#include "ui/events/event_constants.h"

namespace ui {

// GPUEventDispatcher sends native events from GPU to Browser process over
// IPC. All callbacks related to input need to be handled in Browser
// process and hence the events are sent to it over IPC.
class OZONE_WAYLAND_EXPORT GPUEventDispatcher {
 public:
  typedef std::queue<IPC::Message*> DeferredMessages;
  GPUEventDispatcher();
  ~GPUEventDispatcher();

  void ChannelEstablished(IPC::Sender* sender);

  void MotionNotify(float x, float y);
  void ButtonNotify(unsigned handle,
                    ui::EventType type,
                    ui::EventFlags flags,
                    float x,
                    float y);
  void AxisNotify(float x,
                  float y,
                  int xoffset,
                  int yoffset);
  void PointerEnter(unsigned handle, float x, float y);
  void PointerLeave(unsigned handle, float x, float y);
  void KeyNotify(ui::EventType type, unsigned code, int device_id);
  void VirtualKeyNotify(ui::EventType type,
                        uint32_t key,
                        int device_id);
  void TouchNotify(ui::EventType type,
                   float x,
                   float y,
                   int32_t touch_id,
                   uint32_t time_stamp);

  void OutputSizeChanged(unsigned width, unsigned height);
  void WindowResized(unsigned handle,
                     unsigned width,
                     unsigned height);
  void WindowUnminimized(unsigned windowhandle);
  void WindowDeActivated(unsigned windowhandle);
  void WindowActivated(unsigned windowhandle);
  void CloseWidget(unsigned handle);

  void Commit(unsigned handle, const std::string& text);
  void PreeditChanged(unsigned handle,
                      const std::string& text,
                      const std::string& commit);
  void PreeditEnd();
  void PreeditStart();
  void InitializeXKB(base::SharedMemoryHandle fd,
                     uint32_t size);

 private:
  // Posts task to main loop of the thread on which Dispatcher was initialized.
  void Dispatch(IPC::Message* message);
  void Send(IPC::Message* message);
  IPC::Sender* sender_;
  base::MessageLoop* loop_;
  DeferredMessages deferred_messages_;
  // Support weak pointers for attach & detach callbacks.
  base::WeakPtrFactory<GPUEventDispatcher> weak_ptr_factory_;
  DISALLOW_COPY_AND_ASSIGN(GPUEventDispatcher);
};

}  // namespace ui

#endif  // OZONE_PLATFORM_GPU_EVENT_DISPATCHER_H_
