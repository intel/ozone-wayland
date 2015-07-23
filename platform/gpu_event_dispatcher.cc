// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/platform/gpu_event_dispatcher.h"

#include "base/bind.h"
#include "ozone/ui/public/messages.h"

namespace ui {

GPUEventDispatcher::GPUEventDispatcher()
    :sender_(NULL),
     loop_(NULL),
     weak_ptr_factory_(this) {
}

GPUEventDispatcher::~GPUEventDispatcher() {
  while (!deferred_messages_.empty())
    deferred_messages_.pop();
}

void GPUEventDispatcher::ChannelEstablished(IPC::Sender* sender) {
  loop_ = base::MessageLoop::current();
  sender_ = sender;
  while (!deferred_messages_.empty()) {
    Dispatch(deferred_messages_.front());
    deferred_messages_.pop();
  }
}

void GPUEventDispatcher::MotionNotify(float x, float y) {
  Dispatch(new WaylandInput_MotionNotify(x, y));
}

void GPUEventDispatcher::ButtonNotify(unsigned handle,
                                         ui::EventType type,
                                         ui::EventFlags flags,
                                         float x,
                                         float y) {
  Dispatch(new WaylandInput_ButtonNotify(handle, type, flags, x, y));
}

void GPUEventDispatcher::AxisNotify(float x,
                                       float y,
                                       int xoffset,
                                       int yoffset) {
  Dispatch(new WaylandInput_AxisNotify(x, y, xoffset, yoffset));
}

void GPUEventDispatcher::PointerEnter(unsigned handle,
                                         float x,
                                         float y) {
  Dispatch(new WaylandInput_PointerEnter(handle, x, y));
}

void GPUEventDispatcher::PointerLeave(unsigned handle,
                                         float x,
                                         float y) {
  Dispatch(new WaylandInput_PointerLeave(handle, x, y));
}

void GPUEventDispatcher::KeyNotify(ui::EventType type,
                                      unsigned code,
                                      int device_id) {
  Dispatch(new WaylandInput_KeyNotify(type, code, device_id));
}

void GPUEventDispatcher::VirtualKeyNotify(ui::EventType type,
                                             uint32_t key,
                                             int device_id) {
  Dispatch(new WaylandInput_VirtualKeyNotify(type, key, device_id));
}

void GPUEventDispatcher::TouchNotify(ui::EventType type,
                                        float x,
                                        float y,
                                        int32_t touch_id,
                                        uint32_t time_stamp) {
  Dispatch(new WaylandInput_TouchNotify(type, x, y, touch_id, time_stamp));
}

void GPUEventDispatcher::OutputSizeChanged(unsigned width,
                                              unsigned height) {
  Dispatch(new WaylandInput_OutputSize(width, height));
}

void GPUEventDispatcher::WindowResized(unsigned handle,
                                          unsigned width,
                                          unsigned height) {
  Dispatch(new WaylandWindow_Resized(handle, width, height));
}

void GPUEventDispatcher::WindowUnminimized(unsigned handle) {
  Dispatch(new WaylandWindow_Unminimized(handle));
}

void GPUEventDispatcher::WindowDeActivated(unsigned windowhandle) {
  Dispatch(new WaylandWindow_DeActivated(windowhandle));
}

void GPUEventDispatcher::WindowActivated(unsigned windowhandle) {
  Dispatch(new WaylandWindow_Activated(windowhandle));
}

void GPUEventDispatcher::CloseWidget(unsigned handle) {
  Dispatch(new WaylandInput_CloseWidget(handle));
}

void GPUEventDispatcher::Commit(unsigned handle,
                                   const std::string& text) {
  Dispatch(new WaylandInput_Commit(handle, text));
}

void GPUEventDispatcher::PreeditChanged(unsigned handle,
                                           const std::string& text,
                                           const std::string& commit) {
  Dispatch(new WaylandInput_PreeditChanged(handle, text, commit));
}

void GPUEventDispatcher::PreeditEnd() {
  Dispatch(new WaylandInput_PreeditEnd());
}

void GPUEventDispatcher::PreeditStart() {
  Dispatch(new WaylandInput_PreeditStart());
}

void GPUEventDispatcher::InitializeXKB(base::SharedMemoryHandle fd,
                                          uint32_t size) {
  Dispatch(new WaylandInput_InitializeXKB(fd, size));
}

void GPUEventDispatcher::Dispatch(IPC::Message* message) {
  if (!loop_) {
    deferred_messages_.push(message);
    return;
  }

  loop_->task_runner()->PostTask(FROM_HERE,
      base::Bind(&GPUEventDispatcher::Send,
                 weak_ptr_factory_.GetWeakPtr(),
                 message));
}

void GPUEventDispatcher::Send(IPC::Message* message) {
  sender_->Send(message);
}

}  // namespace ui
