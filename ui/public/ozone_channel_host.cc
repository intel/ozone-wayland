// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/public/ozone_channel_host.h"

#include "ozone/ui/events/event_converter_in_process.h"
#include "ozone/ui/events/event_factory_ozone_wayland.h"
#include "ozone/ui/events/remote_state_change_handler.h"
#include "ozone/ui/public/messages.h"

namespace ui {

OzoneChannelHost::OzoneChannelHost()
    : state_handler_(NULL) {
  event_converter_ = new EventConverterInProcess();
  ui::EventFactoryOzoneWayland* event_factory =
      ui::EventFactoryOzoneWayland::GetInstance();
  event_factory->SetEventConverterOzoneWayland(event_converter_);
  state_handler_ = new RemoteStateChangeHandler();
  event_converter_->SetWindowChangeObserver(
    event_factory->GetWindowChangeObserver());
  event_converter_->SetOutputChangeObserver(
    event_factory->GetOutputChangeObserver());
}

OzoneChannelHost::~OzoneChannelHost() {
  delete state_handler_;
}

void OzoneChannelHost::DeleteRemoteStateChangeHandler() {
  delete state_handler_;
  state_handler_ = NULL;
}

void OzoneChannelHost::OnChannelEstablished(int host_id, IPC::Sender* sender) {
  if (state_handler_)
    state_handler_->ChannelEstablished(sender);
}

void OzoneChannelHost::OnChannelDestroyed(int host_id) {
  if (state_handler_)
    state_handler_->ChannelDestroyed();
}

bool OzoneChannelHost::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(OzoneChannelHost, message)
  IPC_MESSAGE_HANDLER(WaylandInput_MotionNotify, OnMotionNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_ButtonNotify, OnButtonNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_TouchNotify, OnTouchNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_AxisNotify, OnAxisNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_PointerEnter, OnPointerEnter)
  IPC_MESSAGE_HANDLER(WaylandInput_PointerLeave, OnPointerLeave)
  IPC_MESSAGE_HANDLER(WaylandInput_KeyNotify, OnKeyNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_VirtualKeyNotify, OnVirtualKeyNotify)
  IPC_MESSAGE_HANDLER(WaylandInput_KeyModifiers, OnKeyModifiers)
  IPC_MESSAGE_HANDLER(WaylandInput_OutputSize, OnOutputSizeChanged)
  IPC_MESSAGE_HANDLER(WaylandInput_CloseWidget, OnCloseWidget)
  IPC_MESSAGE_HANDLER(WaylandWindow_Resized, OnWindowResized)
  IPC_MESSAGE_HANDLER(WaylandWindow_Unminimized, OnWindowUnminimized)
  IPC_MESSAGE_HANDLER(WaylandInput_Commit, OnCommit)
  IPC_MESSAGE_HANDLER(WaylandInput_PreeditChanged, OnPreeditChanged)
  IPC_MESSAGE_HANDLER(WaylandInput_PreeditEnd, OnPreeditEnd)
  IPC_MESSAGE_HANDLER(WaylandInput_PreeditStart, OnPreeditStart)
  IPC_MESSAGE_HANDLER(WaylandInput_InitializeXKB, OnInitializeXKB)
  IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void OzoneChannelHost::OnMotionNotify(float x, float y) {
  event_converter_->MotionNotify(x, y);
}

void OzoneChannelHost::OnButtonNotify(unsigned handle,
                                      ui::EventType type,
                                      ui::EventFlags flags,
                                      float x,
                                      float y) {
  event_converter_->ButtonNotify(handle, type, flags, x, y);
}

void OzoneChannelHost::OnTouchNotify(ui::EventType type,
                                     float x,
                                     float y,
                                     int32_t touch_id,
                                     uint32_t time_stamp) {
  event_converter_->TouchNotify(type, x, y, touch_id, time_stamp);
}

void OzoneChannelHost::OnAxisNotify(float x,
                                    float y,
                                    int xoffset,
                                    int yoffset) {
  event_converter_->AxisNotify(x, y, xoffset, yoffset);
}

void OzoneChannelHost::OnPointerEnter(unsigned handle,
                                      float x,
                                      float y) {
  event_converter_->PointerEnter(handle, x, y);
}

void OzoneChannelHost::OnPointerLeave(unsigned handle,
                                      float x,
                                      float y) {
  event_converter_->PointerLeave(handle, x, y);
}

void OzoneChannelHost::OnKeyNotify(ui::EventType type,
                                   unsigned code) {
  event_converter_->KeyNotify(type, code);
}

void OzoneChannelHost::OnVirtualKeyNotify(ui::EventType type,
                                          uint32_t key,
                                          uint32_t modifiers) {
  event_converter_->VirtualKeyNotify(type, key, modifiers);
}

void OzoneChannelHost::OnKeyModifiers(uint32_t mods_depressed,
                                      uint32_t mods_latched,
                                      uint32_t mods_locked,
                                      uint32_t group) {
  event_converter_->KeyModifiers(mods_depressed,
                                 mods_latched,
                                 mods_locked,
                                 group);
}

void OzoneChannelHost::OnOutputSizeChanged(unsigned width,
                                           unsigned height) {
  event_converter_->OutputSizeChanged(width, height);
}

void OzoneChannelHost::OnCloseWidget(unsigned handle) {
  event_converter_->CloseWidget(handle);
}

void OzoneChannelHost::OnWindowResized(unsigned handle,
                                       unsigned width,
                                       unsigned height) {
  event_converter_->WindowResized(handle, width, height);
}

void OzoneChannelHost::OnWindowUnminimized(unsigned handle) {
  event_converter_->WindowUnminimized(handle);
}

void OzoneChannelHost::OnCommit(unsigned handle, std::string text) {
  event_converter_->Commit(handle, text);
}

void OzoneChannelHost::OnPreeditChanged(unsigned handle, std::string text,
                                        std::string commit) {
  event_converter_->PreeditChanged(handle, text, commit);
}

void OzoneChannelHost::OnPreeditEnd() {
}

void OzoneChannelHost::OnPreeditStart() {
}

void OzoneChannelHost::OnInitializeXKB(base::SharedMemoryHandle fd,
                                       uint32_t size) {
  event_converter_->InitializeXKB(fd, size);
}

}  // namespace ui
