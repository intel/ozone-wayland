// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/platform/ozone_gpu_platform_support.h"

#include "ozone/platform/gpu_event_dispatcher.h"
#include "ozone/platform/messages.h"
#include "ozone/ui/events/event_factory_ozone_wayland.h"
#include "ozone/ui/events/ime_state_change_handler.h"
#include "ozone/wayland/display.h"
#include "ozone/wayland/seat.h"

namespace ui {

OzoneGpuPlatformSupport::OzoneGpuPlatformSupport(
    ozonewayland::WaylandDisplay* display) :sender_(NULL),
                                            display_(display) {
}

OzoneGpuPlatformSupport::~OzoneGpuPlatformSupport() {
}

void OzoneGpuPlatformSupport::OnChannelEstablished(IPC::Sender* sender) {
  display_->GetEventDispatcher()->ChannelEstablished(sender);

  sender_ =  sender;
  for (size_t i = 0; i < handlers_.size(); ++i)
    handlers_[i]->OnChannelEstablished(sender_);
}

void OzoneGpuPlatformSupport::RegisterHandler(GpuPlatformSupport* handler) {
  handlers_.push_back(handler);

  if (sender_)
    handler->OnChannelEstablished(sender_);
}

void OzoneGpuPlatformSupport::UnregisterHandler(GpuPlatformSupport* handler) {
  std::vector<GpuPlatformSupport*>::iterator it =
      std::find(handlers_.begin(), handlers_.end(), handler);
  if (it != handlers_.end())
    handlers_.erase(it);
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

  if (!handled) {
    for (size_t i = 0; i < handlers_.size(); ++i) {
      if (handlers_[i]->OnMessageReceived(message)) {
        return true;
      }
    }
  }

  return handled;
}

void OzoneGpuPlatformSupport::OnWidgetStateChanged(unsigned handleid,
                                                   ui::WidgetState state) {
  display_->SetWidgetState(handleid, state);
}

void OzoneGpuPlatformSupport::OnWidgetTitleChanged(unsigned widget,
                                                   base::string16 title) {
  display_->SetWidgetTitle(widget, title);
}

void OzoneGpuPlatformSupport::OnWidgetCursorChanged(int cursor_type) {
  display_->SetWidgetCursor(cursor_type);
}

void OzoneGpuPlatformSupport::OnWidgetCreate(unsigned widget,
                                             unsigned parent,
                                             unsigned x,
                                             unsigned y,
                                             ui::WidgetType type) {
  display_->CreateWidget(widget, parent, x, y, type);
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

void OzoneGpuPlatformSupport::RelinquishGpuResources(
    const base::Closure& callback) {
  callback.Run();
}

IPC::MessageFilter* OzoneGpuPlatformSupport::GetMessageFilter() {
  return NULL;
}

}  // namespace ui
