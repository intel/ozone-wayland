// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/events/remote_state_change_handler.h"

#include "ipc/ipc_sender.h"
#include "ozone/platform/messages.h"
#include "ozone/platform/ozone_gpu_platform_support_host.h"
#include "ozone/ui/events/event_factory_ozone_wayland.h"
#include "ui/ozone/public/ozone_platform.h"

namespace ui {

RemoteStateChangeHandler::RemoteStateChangeHandler(
    OzoneGpuPlatformSupportHost* proxy)
    : proxy_(proxy),
      isConnected_(false) {
  EventFactoryOzoneWayland::GetInstance()->
      SetIMEStateChangeHandler(this);
  proxy_->AddChannelObserver(this);
}

RemoteStateChangeHandler::~RemoteStateChangeHandler() {
  while (!deferred_messages_.empty())
    deferred_messages_.pop();
}

void RemoteStateChangeHandler::OnChannelEstablished() {
  isConnected_ = true;
  while (!deferred_messages_.empty()) {
    proxy_->Send(deferred_messages_.front());
    deferred_messages_.pop();
  }
}

void RemoteStateChangeHandler::OnChannelDestroyed() {
  isConnected_ = false;
}

void RemoteStateChangeHandler::ResetIme() {
  Send(new WaylandDisplay_ImeReset());
}

void RemoteStateChangeHandler::ImeCaretBoundsChanged(gfx::Rect rect) {
  Send(new WaylandWindow_ImeCaretBoundsChanged(rect));
}

void RemoteStateChangeHandler::HideInputPanel() {
  Send(new WaylandDisplay_HideInputPanel());
}

void RemoteStateChangeHandler::ShowInputPanel() {
  Send(new WaylandDisplay_ShowInputPanel());
}

void RemoteStateChangeHandler::Send(IPC::Message* message) {
  // The GPU process never sends synchronous IPC, so clear the unblock flag.
  // This ensures the message is treated as a synchronous one and helps preserve
  // order. Check set_unblock in ipc_messages.h for explanation.
  message->set_unblock(true);
  if (isConnected_)
    proxy_->Send(message);
  else
    deferred_messages_.push(message);
}

}  // namespace ui
