// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/impl/ipc/display_channel.h"

#include "ozone/impl/ozone_display.h"
#include "content/child/child_thread.h"
#include "content/child/child_process.h"

namespace ozonewayland {

namespace {

content::ChildThread* GetProcessMainThread() {
  content::ChildProcess* process = content::ChildProcess::current();
  return process ? process->main_thread() : NULL;
}

}

OzoneDisplayChannel::OzoneDisplayChannel(unsigned fd)
    : display_fd_(fd),
      route_id_(0),
      mapped_(false)
{
  Register();
}

OzoneDisplayChannel::~OzoneDisplayChannel()
{
  content::ChildThread* thread = GetProcessMainThread();
   if (thread)
     thread->RemoveRoute(route_id_ ? route_id_ : display_fd_);
}

bool OzoneDisplayChannel::OnMessageReceived(
    const IPC::Message& message) {

  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(OzoneDisplayChannel, message)
  IPC_MESSAGE_HANDLER(WaylandMsg_DisplayChannelEstablished, OnEstablishChannel)
  IPC_MESSAGE_HANDLER(WaylandWindow_State, OnWidgetStateChanged)
  IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void OzoneDisplayChannel::OnEstablishChannel(unsigned route_id)
{
  route_id_ = route_id;
  OzoneDisplay::GetInstance()->OnChannelEstablished(route_id_);

  content::ChildThread* thread = GetProcessMainThread();
  if (!thread)
    return;

  thread->RemoveRoute(display_fd_);
  thread->AddRoute(route_id_, this);
}

void OzoneDisplayChannel::Register()
{
  if (!display_fd_ || mapped_)
    return;

  content::ChildThread* thread = GetProcessMainThread();
  if (thread) {
    mapped_ = true;
    thread->Send(new WaylandMsg_EstablishDisplayChannel(display_fd_));
    thread->AddRoute(display_fd_, this);
  }
}

void OzoneDisplayChannel::OnWidgetStateChanged(unsigned handleid,
                                               unsigned state,
                                               unsigned width,
                                               unsigned height)
{
  OzoneDisplay::GetInstance()->OnWidgetStateChanged(handleid, state, width, height);
}

}  // namespace ozonewayland
