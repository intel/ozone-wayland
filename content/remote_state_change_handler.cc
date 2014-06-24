// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/content/remote_state_change_handler.h"

#include "ipc/ipc_sender.h"
#include "ozone/content/messages.h"

namespace content {

RemoteStateChangeHandler::RemoteStateChangeHandler()
    : sender_(NULL) {
  WindowStateChangeHandler::SetInstance(this);
  IMEStateChangeHandler::SetInstance(this);
}

RemoteStateChangeHandler::~RemoteStateChangeHandler() {
  while (!deferred_messages_.empty())
    deferred_messages_.pop();
}

void RemoteStateChangeHandler::ChannelEstablished(IPC::Sender* sender) {
  sender_ = sender;
  while (!deferred_messages_.empty()) {
    sender_->Send(deferred_messages_.front());
    deferred_messages_.pop();
  }
}

void RemoteStateChangeHandler::ChannelDestroyed() {
  sender_ = NULL;
}

void RemoteStateChangeHandler::SetWidgetState(unsigned w,
                                              ui::WidgetState state,
                                              unsigned width,
                                              unsigned height) {
  Send(new WaylandWindow_State(w, state, width, height));
}

void RemoteStateChangeHandler::SetWidgetTitle(unsigned w,
                                              const base::string16& title) {
  Send(new WaylandWindow_Title(w, title));
}

void RemoteStateChangeHandler::SetWidgetCursor(int cursor_type) {
  Send(new WaylandWindow_Cursor(cursor_type));
}

void RemoteStateChangeHandler::SetWidgetAttributes(unsigned widget,
                                                   unsigned parent,
                                                   unsigned x,
                                                   unsigned y,
                                                   ui::WidgetType type) {
  Send(new WaylandWindow_Attributes(widget,
                                    parent,
                                    x,
                                    y,
                                    type));
}

void RemoteStateChangeHandler::ResetIme() {
  Send(new WaylandWindow_ImeReset());
}

void RemoteStateChangeHandler::ImeCaretBoundsChanged(gfx::Rect rect) {
  Send(new WaylandWindow_ImeCaretBoundsChanged(rect));
}

void RemoteStateChangeHandler::HideInputPanel() {
  Send(new WaylandWindow_HideInputPanel());
}

void RemoteStateChangeHandler::ShowInputPanel() {
  Send(new WaylandWindow_ShowInputPanel());
}

void RemoteStateChangeHandler::Send(IPC::Message* message) {
  // The GPU process never sends synchronous IPC, so clear the unblock flag.
  // This ensures the message is treated as a synchronous one and helps preserve
  // order. Check set_unblock in ipc_messages.h for explanation.
  message->set_unblock(true);
  if (!sender_)
    deferred_messages_.push(message);
  else
    sender_->Send(message);
}

}  // namespace content
