// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/impl/ipc/display_channel_host.h"

#include "base/bind.h"
#include "content/public/browser/browser_thread.h"
#include "ozone/impl/ipc/messages.h"
#include "ozone/impl/ozone_display.h"
#include "ozone/ui/events/event_converter_ozone_wayland.h"

namespace ozonewayland {

// This should be same as defined in display_channel.
const int CHANNEL_ROUTE_ID = -0x1;

OzoneDisplayChannelHost::OzoneDisplayChannelHost()
    : IPC::ChannelProxy::MessageFilter(),
      dispatcher_(EventConverterOzoneWayland::GetInstance()),
      channel_(NULL),
      deferred_messages_() {
  WindowStateChangeHandler::SetInstance(this);
}

OzoneDisplayChannelHost::~OzoneDisplayChannelHost() {
  OzoneDisplay::GetInstance()->OnChannelHostDestroyed();
  DCHECK(deferred_messages_.empty());
}

void OzoneDisplayChannelHost::EstablishChannel() {
  if (channel_)
    return;

  content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE,
      base::Bind(base::IgnoreResult(&OzoneDisplayChannelHost::UpdateConnection),
          this));
}

void OzoneDisplayChannelHost::SendWidgetState(unsigned w,
                                              WidgetState state,
                                              unsigned width,
                                              unsigned height) {
  if (!content::BrowserThread::CurrentlyOn(content::BrowserThread::IO)) {
    content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE,
        base::Bind(&OzoneDisplayChannelHost::SendWidgetState,
            base::Unretained(this), w, state, width, height));
    return;
  }

  Send(new WaylandWindow_State(CHANNEL_ROUTE_ID, w, state, width, height));
}

void OzoneDisplayChannelHost::SendWidgetAttributes(unsigned widget,
                                                   unsigned parent,
                                                   unsigned x,
                                                   unsigned y,
                                                   WidgetType type) {
  if (!content::BrowserThread::CurrentlyOn(content::BrowserThread::IO)) {
    content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE,
        base::Bind(&OzoneDisplayChannelHost::SendWidgetAttributes,
            base::Unretained(this), widget, parent, x, y, type));
    return;
  }

  Send(new WaylandWindow_Attributes(CHANNEL_ROUTE_ID,
                                    widget,
                                    parent,
                                    x,
                                    y,
                                    type));
}

void OzoneDisplayChannelHost::SendWidgetTitle(
    unsigned w, const base::string16& title) {
  if (!content::BrowserThread::CurrentlyOn(content::BrowserThread::IO)) {
    content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE,
        base::Bind(&OzoneDisplayChannelHost::SendWidgetTitle,
            base::Unretained(this), w, title));
    return;
  }

  Send(new WaylandWindow_Title(CHANNEL_ROUTE_ID, w, title));
}

void OzoneDisplayChannelHost::SetWidgetState(unsigned w,
                                             WidgetState state,
                                             unsigned width,
                                             unsigned height) {
  if (!content::BrowserThread::CurrentlyOn(content::BrowserThread::IO)) {
    content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE,
        base::Bind(&OzoneDisplayChannelHost::SetWidgetState,
            base::Unretained(this), w, state, width, height));
    return;
  }

  Send(new WaylandWindow_State(CHANNEL_ROUTE_ID, w, state, width, height));
}

void OzoneDisplayChannelHost::SetWidgetTitle(unsigned w,
                                             const base::string16& title) {
  if (!content::BrowserThread::CurrentlyOn(content::BrowserThread::IO)) {
    content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE,
        base::Bind(&OzoneDisplayChannelHost::SetWidgetTitle,
            base::Unretained(this), w, title));
    return;
  }

  Send(new WaylandWindow_Title(CHANNEL_ROUTE_ID, w, title));
}

void OzoneDisplayChannelHost::SetWidgetAttributes(unsigned widget,
                                                  unsigned parent,
                                                  unsigned x,
                                                  unsigned y,
                                                  WidgetType type) {
  if (!content::BrowserThread::CurrentlyOn(content::BrowserThread::IO)) {
    content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE,
        base::Bind(&OzoneDisplayChannelHost::SetWidgetAttributes,
            base::Unretained(this), widget, parent, x, y, type));
    return;
  }

  Send(new WaylandWindow_Attributes(CHANNEL_ROUTE_ID,
                                    widget,
                                    parent,
                                    x,
                                    y,
                                    type));
}


void OzoneDisplayChannelHost::OnChannelEstablished() {
  DCHECK(channel_);
  Send(new WaylandMsg_DisplayChannelEstablished(CHANNEL_ROUTE_ID));
  while (!deferred_messages_.empty()) {
    Send(deferred_messages_.front());
    deferred_messages_.pop();
  }
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

bool OzoneDisplayChannelHost::OnMessageReceived(const IPC::Message& message) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO)) <<
      "Must handle messages that were dispatched to another thread!";

  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(OzoneDisplayChannelHost, message)
  IPC_MESSAGE_HANDLER(WaylandInput_MotionNotify, OnMotionNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_ButtonNotify, OnButtonNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_AxisNotify, OnAxisNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_PointerEnter, OnPointerEnter)
  IPC_MESSAGE_HANDLER(WaylandInput_PointerLeave, OnPointerLeave)
  IPC_MESSAGE_HANDLER(WaylandInput_KeyNotify, OnKeyNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_OutputSize, OnOutputSizeChanged)
  IPC_MESSAGE_HANDLER(WaylandInput_CloseWidget, OnCloseWidget)
  IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void OzoneDisplayChannelHost::OnFilterAdded(IPC::Channel* channel) {
  channel_ = channel;
}

void OzoneDisplayChannelHost::OnChannelClosing() {
  channel_ = NULL;
}

bool OzoneDisplayChannelHost::Send(IPC::Message* message) {
  if (!channel_) {
    deferred_messages_.push(message);
    return true;
  }

  // The GPU process never sends synchronous IPC, so clear the unblock flag.
  // This ensures the message is treated as a synchronous one and helps preserve
  // order. Check set_unblock in ipc_messages.h for explanation.
  message->set_unblock(true);
  return channel_->Send(message);
}

void OzoneDisplayChannelHost::UpdateConnection() {
  content::GpuProcessHost* host = content::GpuProcessHost::Get(
      content::GpuProcessHost::GPU_PROCESS_KIND_SANDBOXED,
      content::CAUSE_FOR_GPU_LAUNCH_BROWSER_STARTUP);

  DCHECK(host);
  host->AddFilter(this);
  OnChannelEstablished();
}

}  // namespace ozonewayland
