// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_CONTENT_REMOTE_STATE_CHANGE_HANDLER_H_
#define OZONE_CONTENT_REMOTE_STATE_CHANGE_HANDLER_H_

#include <queue>

#include "ozone/ui/events/ime_state_change_handler.h"
#include "ozone/ui/events/window_state_change_handler.h"

namespace IPC {
class Message;
class Sender;
}

namespace ui {
// RemoteStateChangeHandler implements WindowStateChangeHandler and
// IMEStateChangeHandler. It is responsible for sending any Ime/Window state
// change events from Browser to GPU process (i.e IPC).

class RemoteStateChangeHandler : public ui::WindowStateChangeHandler,
                                 public ui::IMEStateChangeHandler {
 public:
  typedef std::queue<IPC::Message*> DeferredMessages;
  RemoteStateChangeHandler();
  virtual ~RemoteStateChangeHandler();

  void ChannelEstablished(IPC::Sender* sender);
  void ChannelDestroyed();

  // WindowStateChangeHandler implementation:
  virtual void SetWidgetState(unsigned widget,
                              ui::WidgetState state,
                              unsigned width = 0,
                              unsigned height = 0) OVERRIDE;
  virtual void SetWidgetTitle(unsigned w,
                              const base::string16& title) OVERRIDE;
  virtual void SetWidgetCursor(int cursor_type) OVERRIDE;
  virtual void SetWidgetAttributes(unsigned widget,
                                   unsigned parent,
                                   unsigned x,
                                   unsigned y,
                                   ui::WidgetType type) OVERRIDE;
  virtual void ResetIme() OVERRIDE;
  virtual void ImeCaretBoundsChanged(gfx::Rect rect) OVERRIDE;
  virtual void ShowInputPanel() OVERRIDE;
  virtual void HideInputPanel() OVERRIDE;

 private:
  void Send(IPC::Message* message);

  IPC::Sender* sender_;
  // Messages are not sent by host until connection is established. Host queues
  // all these messages to send after connection is established.
  DeferredMessages deferred_messages_;
  DISALLOW_COPY_AND_ASSIGN(RemoteStateChangeHandler);
};

}  // namespace ui

#endif  // OZONE_CONTENT_EVENTS_REMOTE_STATE_CHANGE_HANDLER_H_
