// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/events/event_factory_ozone_wayland.h"

#include "ozone/ui/events/event_converter_in_process.h"
#include "ozone/ui/events/remote_event_dispatcher.h"

namespace ozonewayland {

// static
EventFactoryOzoneWayland* EventFactoryOzoneWayland::impl_ = NULL;

EventFactoryOzoneWayland::EventFactoryOzoneWayland()
    : event_converter_(NULL),
      observer_(NULL),
      output_observer_(NULL) {
  EventFactoryOzoneWayland::SetInstance(this);
}

EventFactoryOzoneWayland::~EventFactoryOzoneWayland() {
  delete event_converter_;
}

EventFactoryOzoneWayland* EventFactoryOzoneWayland::GetInstance() {
  CHECK(impl_) << "No EventFactoryOzoneWayland implementation set.";
  return impl_;
}

void EventFactoryOzoneWayland::SetInstance(EventFactoryOzoneWayland* impl) {
  CHECK(!impl_) << "Replacing set EventFactoryOzoneWayland implementation.";
  impl_ = impl;
}

void EventFactoryOzoneWayland::SetWindowChangeObserver(
    WindowChangeObserver* observer) {
  observer_ = observer;
  if (event_converter_)
    event_converter_->SetWindowChangeObserver(observer_);
}

void EventFactoryOzoneWayland::SetOutputChangeObserver(
    OutputChangeObserver* observer) {
  output_observer_ = observer;
  if (event_converter_)
    event_converter_->SetOutputChangeObserver(output_observer_);
}

OutputChangeObserver*
EventFactoryOzoneWayland::GetOutputChangeObserver() const {
  return output_observer_;
}

EventConverterOzoneWayland* EventFactoryOzoneWayland::EventConverter() const {
  CHECK(impl_) << "EventConverterOzoneWayland is not initialized yet.";
  return event_converter_;
}

void EventFactoryOzoneWayland::StartProcessingEvents() {
  DCHECK(!event_converter_);
  event_converter_ = new EventConverterInProcess();
  event_converter_->SetWindowChangeObserver(observer_);
  event_converter_->SetOutputChangeObserver(output_observer_);
}

void EventFactoryOzoneWayland::StartProcessingRemoteEvents() {
  DCHECK(!event_converter_);
  event_converter_ = new RemoteEventDispatcher();
}

}  // namespace ozonewayland
