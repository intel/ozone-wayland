// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/events/event_factory_ozone_wayland.h"

namespace ui {

// static
EventFactoryOzoneWayland* EventFactoryOzoneWayland::impl_ = NULL;

EventFactoryOzoneWayland::EventFactoryOzoneWayland()
    : observer_(NULL),
      ime_observer_(NULL),
      output_observer_(NULL),
      state_change_handler_(NULL),
      ime_state_handler_(NULL) {
  impl_ = this;
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

void EventFactoryOzoneWayland::SetEventConverter(
    EventConverterOzoneWayland* event_converter) {
  event_converter_ = event_converter;
}

EventConverterOzoneWayland*
EventFactoryOzoneWayland::GetEventConverter() const {
  return event_converter_;
}

void EventFactoryOzoneWayland::SetWindowStateChangeHandler(
    WindowStateChangeHandler* handler) {
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
