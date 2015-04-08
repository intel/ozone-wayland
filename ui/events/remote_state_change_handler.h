// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_EVENTS_REMOTE_STATE_CHANGE_HANDLER_H_
#define OZONE_UI_EVENTS_REMOTE_STATE_CHANGE_HANDLER_H_

#include <queue>

#include "ozone/ui/events/ime_state_change_handler.h"
#include "ozone/ui/events/window_state_change_handler.h"
#include "ui/ozone/platform/drm/host/channel_observer.h"

namespace IPC {
class Message;
class Sender;
}

namespace ui {
class DrmGpuPlatformSupportHost;
// RemoteStateChangeHandler implements WindowStateChangeHandler and
// IMEStateChangeHandler. It is responsible for sending any Ime/Window state
// change events from Browser to GPU process (i.e IPC).

class RemoteStateChangeHandler : public ui::WindowStateChangeHandler,
                                 public ui::IMEStateChangeHandler,
                                 public ui::ChannelObserver {
 public:
  typedef std::queue<IPC::Message*> DeferredMessages;
  explicit RemoteStateChangeHandler(DrmGpuPlatformSupportHost* proxy);
  ~RemoteStateChangeHandler() override;

  // ChannelObserver implementation:
  void OnChannelEstablished() override;
  void OnChannelDestroyed() override;

  // WindowStateChangeHandler implementation:
  void SetWidgetState(unsigned widget,
                      ui::WidgetState state) override;
  void SetWidgetTitle(unsigned w,
                      const base::string16& title) override;
  void SetWidgetCursor(int cursor_type) override;
  void CreateWidget(unsigned widget,
                    unsigned parent,
                    unsigned x,
                    unsigned y,
                    ui::WidgetType type) override;
  void ResetIme() override;
  void ImeCaretBoundsChanged(gfx::Rect rect) override;
  void ShowInputPanel() override;
  void HideInputPanel() override;

 private:
  void Send(IPC::Message* message);

  IPC::Sender* sender_;
  DrmGpuPlatformSupportHost* proxy_;
  bool isConnected_;
  // Messages are not sent by host until connection is established. Host queues
  // all these messages to send after connection is established.
  DeferredMessages deferred_messages_;
  DISALLOW_COPY_AND_ASSIGN(RemoteStateChangeHandler);
};

}  // namespace ui

#endif  // OZONE_UI_EVENTS_REMOTE_STATE_CHANGE_HANDLER_H_
