// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/content/ozone_channel_host.h"

#include <string>

#include "base/bind.h"
#include "content/browser/gpu/browser_gpu_channel_host_factory.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/common/process_type.h"
#include "ozone/content/messages.h"
#include "ozone/content/remote_state_change_handler.h"
#include "ozone/ui/events/event_factory_ozone_wayland.h"

namespace content {

OzoneChannelHost::OzoneChannelHost()
    : state_handler_(NULL) {
  dispatcher_ = ui::EventFactoryOzoneWayland::GetInstance()->EventConverter();
  BrowserChildProcessObserver::Add(this);
  EstablishChannel();
}

OzoneChannelHost::~OzoneChannelHost() {
  BrowserChildProcessObserver::Remove(this);
  delete state_handler_;
}

void OzoneChannelHost::OnMotionNotify(float x, float y) {
  dispatcher_->MotionNotify(x, y);
}

void OzoneChannelHost::OnButtonNotify(unsigned handle,
                                             ui::EventType type,
                                             ui::EventFlags flags,
                                             float x,
                                             float y) {
  dispatcher_->ButtonNotify(handle, type, flags, x, y);
}

void OzoneChannelHost::OnTouchNotify(ui::EventType type,
                                     float x,
                                     float y,
                                     int32_t touch_id,
                                     uint32_t time_stamp) {
  dispatcher_->TouchNotify(type, x, y, touch_id, time_stamp);
}

void OzoneChannelHost::OnAxisNotify(float x,
                                           float y,
                                           int xoffset,
                                           int yoffset) {
  dispatcher_->AxisNotify(x, y, xoffset, yoffset);
}

void OzoneChannelHost::OnPointerEnter(unsigned handle,
                                             float x,
                                             float y) {
  dispatcher_->PointerEnter(handle, x, y);
}

void OzoneChannelHost::OnPointerLeave(unsigned handle,
                                             float x,
                                             float y) {
  dispatcher_->PointerLeave(handle, x, y);
}

void OzoneChannelHost::OnKeyNotify(ui::EventType type,
                                          unsigned code,
                                          unsigned modifiers) {
  dispatcher_->KeyNotify(type, code, modifiers);
}

void OzoneChannelHost::OnOutputSizeChanged(unsigned width,
                                                  unsigned height) {
  dispatcher_->OutputSizeChanged(width, height);
}

void OzoneChannelHost::OnCloseWidget(unsigned handle) {
  dispatcher_->CloseWidget(handle);
}

void OzoneChannelHost::OnWindowResized(unsigned handle,
                                              unsigned width,
                                              unsigned height) {
  dispatcher_->WindowResized(handle, width, height);
}

void OzoneChannelHost::OnCommit(unsigned handle, std::string text) {
  dispatcher_->Commit(handle, text);
}

void OzoneChannelHost::OnPreeditChanged(unsigned handle, std::string text,
                                        std::string commit) {
  dispatcher_->PreeditChanged(handle, text, commit);
}

void OzoneChannelHost::OnPreeditEnd() {
}

void OzoneChannelHost::OnPreeditStart() {
}

void OzoneChannelHost::BrowserChildProcessHostConnected(
  const ChildProcessData& data) {
  // we observe GPU process being forked or re-spawned for adding ourselves as
  // an ipc filter and listen to any relevant messages coming from GpuProcess
  // side.
  if (data.process_type == PROCESS_TYPE_GPU)
    EstablishChannel();
}

void OzoneChannelHost::BrowserChildProcessHostDisconnected(
  const ChildProcessData& data) {
  if (data.process_type == PROCESS_TYPE_GPU) {
    if (state_handler_) {
      delete state_handler_;
      state_handler_ = NULL;
    }
  }
}

void OzoneChannelHost::BrowserChildProcessCrashed(
  const ChildProcessData& data) {
  if (data.process_type == PROCESS_TYPE_GPU) {
    if (state_handler_) {
      delete state_handler_;
      state_handler_ = NULL;
    }
  }
}

void OzoneChannelHost::OnMessageReceived(const IPC::Message& message) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO)) <<
      "Must handle messages that were dispatched to another thread!";

  IPC_BEGIN_MESSAGE_MAP(OzoneChannelHost, message)
  IPC_MESSAGE_HANDLER(WaylandInput_MotionNotify, OnMotionNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_ButtonNotify, OnButtonNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_TouchNotify, OnTouchNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_AxisNotify, OnAxisNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_PointerEnter, OnPointerEnter)
  IPC_MESSAGE_HANDLER(WaylandInput_PointerLeave, OnPointerLeave)
  IPC_MESSAGE_HANDLER(WaylandInput_KeyNotify, OnKeyNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_OutputSize, OnOutputSizeChanged)
  IPC_MESSAGE_HANDLER(WaylandInput_CloseWidget, OnCloseWidget)
  IPC_MESSAGE_HANDLER(WaylandWindow_Resized, OnWindowResized)
  IPC_MESSAGE_HANDLER(WaylandInput_Commit, OnCommit)
  IPC_MESSAGE_HANDLER(WaylandInput_PreeditChanged, OnPreeditChanged)
  IPC_MESSAGE_HANDLER(WaylandInput_PreeditEnd, OnPreeditEnd)
  IPC_MESSAGE_HANDLER(WaylandInput_PreeditStart, OnPreeditStart)

  IPC_END_MESSAGE_MAP()
}

void OzoneChannelHost::EstablishChannel() {
  if (state_handler_)
    return;

  state_handler_ = new RemoteStateChangeHandler();
  BrowserThread::PostTask(BrowserThread::IO, FROM_HERE,
      base::Bind(&OzoneChannelHost::UpdateConnection,
          base::Unretained(this)));
}

void OzoneChannelHost::UpdateConnection() {
  BrowserGpuChannelHostFactory* hostFactory =
      BrowserGpuChannelHostFactory::instance();
  DCHECK(hostFactory);
  const uint32 kMessagesToFilter[] = { WaylandInput_MotionNotify::ID,
                                       WaylandInput_ButtonNotify::ID,
                                       WaylandInput_AxisNotify::ID,
                                       WaylandInput_PointerEnter::ID,
                                       WaylandInput_PointerLeave::ID,
                                       WaylandInput_KeyNotify::ID,
                                       WaylandInput_OutputSize::ID,
                                       WaylandInput_CloseWidget::ID,
                                       WaylandWindow_Resized::ID,
                                       WaylandInput_TouchNotify::ID,
                                       WaylandInput_Commit::ID,
                                       WaylandInput_PreeditChanged::ID,
                                       WaylandInput_PreeditEnd::ID,
                                       WaylandInput_PreeditStart::ID,
                                       };
  scoped_refptr<base::SingleThreadTaskRunner> compositor_thread_task_runner =
      base::MessageLoopProxy::current();
  hostFactory->SetHandlerForControlMessages(
      kMessagesToFilter, arraysize(kMessagesToFilter),
          base::Bind(&OzoneChannelHost::OnMessageReceived,
              base::Unretained(this)), compositor_thread_task_runner.get());
}

}  // namespace content
