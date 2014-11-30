// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/events/event_factory_ozone_wayland.h"

#include "base/bind.h"
#include "ozone/ui/public/ozone_channel_host.h"


namespace ui {

// static
EventFactoryOzoneWayland* EventFactoryOzoneWayland::impl_ = NULL;

EventFactoryOzoneWayland::EventFactoryOzoneWayland(OzoneChannelHost* host)
    : event_converter_(NULL),
      observer_(NULL),
      ime_observer_(NULL),
      output_observer_(NULL),
      state_change_handler_(NULL),
      ime_state_handler_(NULL),
      host_(host) {
  impl_ = this;
  if (host_)
    host_->Initialize();
}

EventFactoryOzoneWayland::~EventFactoryOzoneWayland() {
  impl_ = NULL;
}

EventFactoryOzoneWayland* EventFactoryOzoneWayland::GetInstance() {
  CHECK(impl_) << "No EventFactoryOzoneWayland implementation set.";
  return impl_;
}

void EventFactoryOzoneWayland::SetWindowChangeObserver(
    WindowChangeObserver* observer) {
  observer_ = observer;
}

void EventFactoryOzoneWayland::SetIMEChangeObserver(
    IMEChangeObserver* observer) {
  ime_observer_ = observer;
}

IMEChangeObserver*
EventFactoryOzoneWayland::GetIMEChangeObserver() const {
  return ime_observer_;
}

WindowChangeObserver*
EventFactoryOzoneWayland::GetWindowChangeObserver() const {
  return observer_;
}

void EventFactoryOzoneWayland::SetOutputChangeObserver(
    OutputChangeObserver* observer) {
  output_observer_ = observer;
}

OutputChangeObserver*
EventFactoryOzoneWayland::GetOutputChangeObserver() const {
  return output_observer_;
}

void EventFactoryOzoneWayland::SetEventConverterOzoneWayland(
    EventConverterOzoneWayland* converter) {
  event_converter_ = converter;
}

EventConverterOzoneWayland* EventFactoryOzoneWayland::EventConverter() const {
  CHECK(impl_) << "EventConverterOzoneWayland is not initialized yet.";
  return event_converter_;
}

void EventFactoryOzoneWayland::SetWindowStateChangeHandler(
    WindowStateChangeHandler* handler) {
  if (host_ && host_->GetStateChangeHandler() &&
      handler != host_->GetStateChangeHandler()) {
    host_->ReleaseRemoteStateChangeHandler();
  }

  state_change_handler_ = handler;
}

WindowStateChangeHandler*
EventFactoryOzoneWayland::GetWindowStateChangeHandler() const {
  return state_change_handler_;
}

void EventFactoryOzoneWayland::SetIMEStateChangeHandler(
    IMEStateChangeHandler* handler) {
  ime_state_handler_ = handler;
}

IMEStateChangeHandler*
EventFactoryOzoneWayland::GetImeStateChangeHandler() const {
  return ime_state_handler_;
}

}  // namespace ui
