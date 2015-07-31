// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/platform/ozone_gpu_platform_support.h"

#include "ozone/platform/gpu_event_dispatcher.h"
#include "ozone/platform/messages.h"
#include "ozone/wayland/display.h"

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
  IPC_MESSAGE_HANDLER(WaylandDisplay_AddRegion, OnDisplayAddRegion)
  IPC_MESSAGE_HANDLER(WaylandDisplay_SubRegion, OnDisplaySubRegion)
  IPC_MESSAGE_HANDLER(WaylandDisplay_CursorSet, OnDisplaySetCursor)
  IPC_MESSAGE_HANDLER(WaylandDisplay_MoveCursor, OnDisplayMoveCursor)
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

void OzoneGpuPlatformSupport::OnWidgetCreate(unsigned widget,
                                             unsigned parent,
                                             unsigned x,
                                             unsigned y,
                                             ui::WidgetType type) {
  display_->CreateWidget(widget, parent, x, y, type);
}

void OzoneGpuPlatformSupport::OnDisplayAddRegion(unsigned widget,
                                                 int left,
                                                 int top,
                                                 int right,
                                                 int bottom) {
  display_->AddRegion(widget, left, top, right, bottom);
}

void OzoneGpuPlatformSupport::OnDisplaySubRegion(unsigned widget,
                                                 int left,
                                                 int top,
                                                 int right,
                                                 int bottom) {
  display_->SubRegion(widget, left, top, right, bottom);
}

void OzoneGpuPlatformSupport::OnDisplaySetCursor(
    const std::vector<SkBitmap>& bitmaps, const gfx::Point& location) {
  display_->SetCursorBitmap(bitmaps, location);
}

void OzoneGpuPlatformSupport::OnDisplayMoveCursor(const gfx::Point& location) {
  display_->MoveCursor(location);
}

void OzoneGpuPlatformSupport::OnWidgetImeReset() {
  display_->ResetIme();
}

void OzoneGpuPlatformSupport::OnWidgetShowInputPanel() {
  display_->ShowInputPanel();
}

void OzoneGpuPlatformSupport::OnWidgetHideInputPanel() {
  display_->HideInputPanel();
}

void OzoneGpuPlatformSupport::RelinquishGpuResources(
    const base::Closure& callback) {
  callback.Run();
}

IPC::MessageFilter* OzoneGpuPlatformSupport::GetMessageFilter() {
  return NULL;
}

}  // namespace ui
