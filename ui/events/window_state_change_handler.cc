// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/events/window_state_change_handler.h"

#include "base/logging.h"

namespace ozonewayland {

// static
WindowStateChangeHandler* WindowStateChangeHandler::impl_ = NULL;

WindowStateChangeHandler::WindowStateChangeHandler() {
}

WindowStateChangeHandler::~WindowStateChangeHandler() {
}

WindowStateChangeHandler* WindowStateChangeHandler::GetInstance() {
  CHECK(impl_) << "No WindowStateChangeHandler implementation set.";
  return impl_;
}

void WindowStateChangeHandler::SetInstance(WindowStateChangeHandler* impl) {
  CHECK(!impl_) << "Replacing set WindowStateChangeHandler implementation.";
  impl_ = impl;
}

}  // namespace ozonewayland
