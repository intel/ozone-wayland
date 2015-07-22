// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/public/ozone_gpu_platform_support.h"

#include "ozone/ui/events/event_converter_ozone_wayland.h"
#include "ozone/ui/events/event_factory_ozone_wayland.h"
#include "ozone/ui/events/ime_state_change_handler.h"
#include "ozone/ui/events/window_state_change_handler.h"
#include "ozone/ui/public/messages.h"

namespace ui {

OzoneGpuPlatformSupport::OzoneGpuPlatformSupport() {
}

OzoneGpuPlatformSupport::~OzoneGpuPlatformSupport() {
}

void OzoneGpuPlatformSupport::OnChannelEstablished(IPC::Sender* sender) {
  ui::EventFactoryOzoneWayland::GetInstance()->GetEventConverter()->
      ChannelEstablished(sender);
}

bool OzoneGpuPlatformSupport::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(OzoneGpuPlatformSupport, message)
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

void OzoneGpuPlatformSupport::OnWidgetStateChanged(unsigned handleid,
                                        ui::WidgetState state) {
  EventFactoryOzoneWayland::GetInstance()->GetWindowStateChangeHandler()->
      SetWidgetState(handleid, state);
}

void OzoneGpuPlatformSupport::OnWidgetTitleChanged(unsigned widget,
                                        base::string16 title) {
  EventFactoryOzoneWayland::GetInstance()->GetWindowStateChangeHandler()->
      SetWidgetTitle(widget, title);
}

void OzoneGpuPlatformSupport::OnWidgetCursorChanged(int cursor_type) {
  EventFactoryOzoneWayland::GetInstance()->GetWindowStateChangeHandler()->
      SetWidgetCursor(cursor_type);
}

void OzoneGpuPlatformSupport::OnWidgetCreate(unsigned widget,
                                  unsigned parent,
                                  unsigned x,
                                  unsigned y,
                                  ui::WidgetType type) {
  EventFactoryOzoneWayland::GetInstance()->GetWindowStateChangeHandler()->
      CreateWidget(widget, parent, x, y, type);
}

void OzoneGpuPlatformSupport::OnWidgetImeReset() {
  EventFactoryOzoneWayland::GetInstance()->GetImeStateChangeHandler()->
      ResetIme();
}

void OzoneGpuPlatformSupport::OnWidgetShowInputPanel() {
  EventFactoryOzoneWayland::GetInstance()->GetImeStateChangeHandler()->
      ShowInputPanel();
}

void OzoneGpuPlatformSupport::OnWidgetHideInputPanel() {
  EventFactoryOzoneWayland::GetInstance()->GetImeStateChangeHandler()->
      HideInputPanel();
}

void OzoneGpuPlatformSupport::RelinquishGpuResources(const base::Closure& callback) {
  callback.Run();
}

IPC::MessageFilter* OzoneGpuPlatformSupport::GetMessageFilter() {
  return NULL;
}


}  // namespace ui
