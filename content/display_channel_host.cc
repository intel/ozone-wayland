// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/content/display_channel_host.h"

#include "base/bind.h"
#include "content/browser/gpu/browser_gpu_channel_host_factory.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/common/process_type.h"
#include "ozone/content/messages.h"
#include "ozone/ui/events/event_factory_ozone_wayland.h"
#include "ozone/ui/events/remote_state_change_handler.h"

namespace content {

OzoneDisplayChannelHost::OzoneDisplayChannelHost()
    : state_handler_(NULL) {
  dispatcher_ = ui::EventFactoryOzoneWayland::GetInstance()->EventConverter();
  BrowserChildProcessObserver::Add(this);
  EstablishChannel();
}

OzoneDisplayChannelHost::~OzoneDisplayChannelHost() {
  BrowserChildProcessObserver::Remove(this);
  delete state_handler_;
}

void OzoneDisplayChannelHost::OnMotionNotify(float x, float y) {
  dispatcher_->MotionNotify(x, y);
}

void OzoneDisplayChannelHost::OnButtonNotify(unsigned handle,
                                             ui::EventType type,
                                             ui::EventFlags flags,
                                             float x,
                                             float y) {
  dispatcher_->ButtonNotify(handle, type, flags, x, y);
}

void OzoneDisplayChannelHost::OnAxisNotify(float x,
                                           float y,
                                           int xoffset,
                                           int yoffset) {
  dispatcher_->AxisNotify(x, y, xoffset, yoffset);
}

void OzoneDisplayChannelHost::OnPointerEnter(unsigned handle,
                                             float x,
                                             float y) {
  dispatcher_->PointerEnter(handle, x, y);
}

void OzoneDisplayChannelHost::OnPointerLeave(unsigned handle,
                                             float x,
                                             float y) {
  dispatcher_->PointerLeave(handle, x, y);
}

void OzoneDisplayChannelHost::OnKeyNotify(ui::EventType type,
                                          unsigned code,
                                          unsigned modifiers) {
  dispatcher_->KeyNotify(type, code, modifiers);
}

void OzoneDisplayChannelHost::OnOutputSizeChanged(unsigned width,
                                                  unsigned height) {
  dispatcher_->OutputSizeChanged(width, height);
}

void OzoneDisplayChannelHost::OnCloseWidget(unsigned handle) {
  dispatcher_->CloseWidget(handle);
}

void OzoneDisplayChannelHost::OnWindowResized(unsigned handle,
                                              unsigned width,
                                              unsigned height) {
  dispatcher_->WindowResized(handle, width, height);
}

void OzoneDisplayChannelHost::BrowserChildProcessHostConnected(
  const content::ChildProcessData& data) {
  // we observe GPU process being forked or re-spawned for adding ourselves as
  // an ipc filter and listen to any relevant messages coming from GpuProcess
  // side.
  if (data.process_type == content::PROCESS_TYPE_GPU)
    EstablishChannel();
}

void OzoneDisplayChannelHost::BrowserChildProcessHostDisconnected(
  const content::ChildProcessData& data) {
  if (data.process_type == content::PROCESS_TYPE_GPU) {
    if (state_handler_) {
      delete state_handler_;
      state_handler_ = NULL;
    }
  }
}

void OzoneDisplayChannelHost::BrowserChildProcessCrashed(
  const content::ChildProcessData& data) {
  if (data.process_type == content::PROCESS_TYPE_GPU) {
    if (state_handler_) {
      delete state_handler_;
      state_handler_ = NULL;
    }
  }
}

void OzoneDisplayChannelHost::OnMessageReceived(const IPC::Message& message) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO)) <<
      "Must handle messages that were dispatched to another thread!";

  IPC_BEGIN_MESSAGE_MAP(OzoneDisplayChannelHost, message)
  IPC_MESSAGE_HANDLER(WaylandInput_MotionNotify, OnMotionNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_ButtonNotify, OnButtonNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_AxisNotify, OnAxisNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_PointerEnter, OnPointerEnter)
  IPC_MESSAGE_HANDLER(WaylandInput_PointerLeave, OnPointerLeave)
  IPC_MESSAGE_HANDLER(WaylandInput_KeyNotify, OnKeyNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_OutputSize, OnOutputSizeChanged)
  IPC_MESSAGE_HANDLER(WaylandInput_CloseWidget, OnCloseWidget)
  IPC_MESSAGE_HANDLER(WaylandWindow_Resized, OnWindowResized)
  IPC_END_MESSAGE_MAP()
}

void OzoneDisplayChannelHost::EstablishChannel() {
  if (state_handler_)
    return;

  state_handler_ = new ui::RemoteStateChangeHandler();
  content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE,
      base::Bind(&OzoneDisplayChannelHost::UpdateConnection,
          base::Unretained(this)));
}

void OzoneDisplayChannelHost::UpdateConnection() {
  content::BrowserGpuChannelHostFactory* hostFactory =
      content::BrowserGpuChannelHostFactory::instance();
  DCHECK(hostFactory);
  const uint32 kMessagesToFilter[] = { WaylandInput_MotionNotify::ID,
                                       WaylandInput_ButtonNotify::ID,
                                       WaylandInput_AxisNotify::ID,
                                       WaylandInput_PointerEnter::ID,
                                       WaylandInput_PointerLeave::ID,
                                       WaylandInput_KeyNotify::ID,
                                       WaylandInput_OutputSize::ID,
                                       WaylandInput_CloseWidget::ID,
                                       WaylandWindow_Resized::ID };
  scoped_refptr<base::SingleThreadTaskRunner> compositor_thread_task_runner =
      base::MessageLoopProxy::current();
  hostFactory->SetHandlerForControlMessages(
      kMessagesToFilter, arraysize(kMessagesToFilter),
          base::Bind(&OzoneDisplayChannelHost::OnMessageReceived,
              base::Unretained(this)), compositor_thread_task_runner.get());
}

}  // namespace content
