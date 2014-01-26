// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/events/event_converter_ozone_wayland.h"

#include "base/bind.h"

namespace ozonewayland {

// static
EventConverterOzoneWayland* EventConverterOzoneWayland::impl_ = NULL;

EventConverterOzoneWayland::EventConverterOzoneWayland()
    : loop_(base::MessageLoop::current()) {
  DCHECK(loop_);
  EventConverterOzoneWayland::SetInstance(this);
}

EventConverterOzoneWayland::~EventConverterOzoneWayland() {
}

EventConverterOzoneWayland* EventConverterOzoneWayland::GetInstance() {
  CHECK(impl_) << "No EventConverterOzoneWayland implementation set.";
  return impl_;
}

void EventConverterOzoneWayland::SetInstance(EventConverterOzoneWayland* impl) {
  CHECK(!impl_) << "Replacing set EventConverterOzoneWayland implementation.";
  impl_ = impl;
}

void EventConverterOzoneWayland::SetWindowChangeObserver(
    WindowChangeObserver* observer) {
}

void EventConverterOzoneWayland::SetOutputChangeObserver(
    OutputChangeObserver* observer) {
}

void EventConverterOzoneWayland::PostTaskOnMainLoop(const base::Closure& task) {
  DCHECK(loop_);
  loop_->message_loop_proxy()->PostTask(FROM_HERE, task);
}

}  // namespace ozonewayland
