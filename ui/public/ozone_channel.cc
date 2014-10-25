// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/public/ozone_channel.h"

#include "ozone/ui/events/event_factory_ozone_wayland.h"
#include "ozone/ui/events/ime_state_change_handler.h"
#include "ozone/ui/events/remote_event_dispatcher.h"
#include "ozone/ui/events/window_state_change_handler.h"
#include "ozone/ui/public/messages.h"

namespace ui {

OzoneChannel::OzoneChannel() : event_converter_(NULL) {
}

OzoneChannel::~OzoneChannel() {
}

void OzoneChannel::InitializeRemoteDispatcher() {
  event_converter_ = new RemoteEventDispatcher();
  ui::EventFactoryOzoneWayland::GetInstance()->
      SetEventConverterOzoneWayland(event_converter_);
}

void OzoneChannel::OnChannelEstablished(IPC::Sender* sender) {
  if (event_converter_)
    event_converter_->ChannelEstablished(sender);
}

bool OzoneChannel::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(OzoneChannel, message)
  IPC_MESSAGE_HANDLER(WaylandWindow_State, OnWidgetStateChanged)
  IPC_MESSAGE_HANDLER(WaylandWindow_Create, OnWidgetCreate)
  IPC_MESSAGE_HANDLER(WaylandWindow_Title, OnWidgetTitleChanged)
  IPC_MESSAGE_HANDLER(WaylandWindow_Cursor, OnWidgetCursorChanged)
  IPC_MESSAGE_HANDLER(WaylandWindow_ImeReset, OnWidgetImeReset)
  IPC_MESSAGE_HANDLER(WaylandWindow_ShowInputPanel, OnWidgetShowInputPanel)
  IPC_MESSAGE_HANDLER(WaylandWindow_HideInputPanel, OnWidgetHideInputPanel)
  IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
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

void OzoneChannel::OnWidgetCursorChanged(int cursor_type) {
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetCursor(cursor_type);
}

void OzoneChannel::OnWidgetCreate(unsigned widget,
                                  unsigned parent,
                                  unsigned x,
                                  unsigned y,
                                  ui::WidgetType type) {
  ui::WindowStateChangeHandler::GetInstance()->CreateWidget(widget,
                                                            parent,
                                                            x,
                                                            y,
                                                            type);
}

void OzoneChannel::OnWidgetImeReset() {
  ui::IMEStateChangeHandler::GetInstance()->ResetIme();
}

void OzoneChannel::OnWidgetShowInputPanel() {
  ui::IMEStateChangeHandler::GetInstance()->ShowInputPanel();
}

void OzoneChannel::OnWidgetHideInputPanel() {
  ui::IMEStateChangeHandler::GetInstance()->HideInputPanel();
}

}  // namespace ui
