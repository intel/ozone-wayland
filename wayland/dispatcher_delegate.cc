// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/dispatcher_delegate.h"

#include "base/bind.h"
#include "ozone/wayland/input/kbd_conversion.h"

namespace ozonewayland {

WaylandDispatcherDelegate::WaylandDispatcherDelegate()
    : loop_(NULL),
      ignore_task_(true) {
}

WaylandDispatcherDelegate::~WaylandDispatcherDelegate() {
}

void WaylandDispatcherDelegate::SetActive(bool active) {
  if (active) {
    loop_ = base::MessageLoop::current();
    if (loop_)
      ignore_task_ = false;
    DCHECK(!ignore_task_);
  } else {
    ignore_task_ = true;
      loop_ = NULL;
  }
}

void WaylandDispatcherDelegate::OutputSizeChanged(unsigned width,
                                                  unsigned height) {
}

void WaylandDispatcherDelegate::SetWindowChangeObserver(
    WindowChangeObserver* observer) {
}

void WaylandDispatcherDelegate::PostTaskOnMainLoop(const base::Closure& task) {
  if (ignore_task_)
    return;

  DCHECK(loop_);
  loop_->message_loop_proxy()->PostTask(FROM_HERE, task);
}

}  // namespace ozonewayland
