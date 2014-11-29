// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/events/event_converter_ozone_wayland.h"

#include "base/bind.h"

namespace ui {

EventConverterOzoneWayland::EventConverterOzoneWayland()
    : loop_(NULL) {
}

EventConverterOzoneWayland::~EventConverterOzoneWayland() {
}

void EventConverterOzoneWayland::PostTaskOnMainLoop(const base::Closure& task) {
  DCHECK(loop_);
  loop_->message_loop_proxy()->PostTask(FROM_HERE, task);
}

}  // namespace ui
