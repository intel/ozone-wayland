// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/content/ozone_channel.h"

#include "content/child/child_process.h"
#include "content/child/child_thread.h"
#include "ozone/content/messages.h"
#include "ozone/content/remote_event_dispatcher.h"
#include "ozone/ui/events/event_factory_ozone_wayland.h"
#include "ozone/ui/events/window_state_change_handler.h"

namespace content {
// GpuChannelManager generates unique routeid for every new
// ImageTransportSurface. In Ozone-Wayland, we register a routeid between
// DisplayChannel and ChannelHost. Therefore, we hardcore our own routeid with a
// unique negitive value to avoid any conflicts from the GpuChannelManager ones.
#define WAYLAND_ROUTE_ID -0x1

namespace {

content::ChildThread* GetProcessMainThread() {
  content::ChildProcess* process = content::ChildProcess::current();
  DCHECK(process && process->main_thread());
  return process->main_thread();
}

}

OzoneChannel::OzoneChannel() {
  event_converter_ = new RemoteEventDispatcher();
  ui::EventFactoryOzoneWayland::GetInstance()->
      SetEventConverterOzoneWayland(event_converter_);
}

OzoneChannel::~OzoneChannel() {
  ChildThread* thread = GetProcessMainThread();
  thread->RemoveRoute(WAYLAND_ROUTE_ID);
}

bool OzoneChannel::OnMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(OzoneChannel, message)
  IPC_MESSAGE_HANDLER(WaylandWindow_State, OnWidgetStateChanged)
  IPC_MESSAGE_HANDLER(WaylandWindow_Attributes, OnWidgetAttributesChanged)
  IPC_MESSAGE_HANDLER(WaylandWindow_Title, OnWidgetTitleChanged)
  IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void OzoneChannel::Register() {
  ChildThread* thread = GetProcessMainThread();
  thread->AddRoute(WAYLAND_ROUTE_ID, this);
}

void OzoneChannel::OnWidgetStateChanged(unsigned handleid,
                                               ui::WidgetState state,
                                               unsigned width,
                                               unsigned height) {
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(handleid,
                                                              state,
                                                              width,
                                                              height);
}

void OzoneChannel::OnWidgetTitleChanged(unsigned widget,
                                               base::string16 title) {
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetTitle(widget, title);
}

void OzoneChannel::OnWidgetAttributesChanged(unsigned widget,
                                                    unsigned parent,
                                                    unsigned x,
                                                    unsigned y,
                                                    ui::WidgetType type) {
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetAttributes(widget,
                                                                   parent,
                                                                   x,
                                                                   y,
                                                                   type);
}

}  // namespace content
