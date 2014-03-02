// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/events/ime_state_change_handler.h"

#include "base/logging.h"

namespace ui {

// static
IMEStateChangeHandler* IMEStateChangeHandler::impl_ = NULL;

IMEStateChangeHandler::IMEStateChangeHandler() {
}

IMEStateChangeHandler::~IMEStateChangeHandler() {
}

IMEStateChangeHandler* IMEStateChangeHandler::GetInstance() {
  CHECK(impl_) << "No IMEStateChangeHandler implementation set.";
  return impl_;
}

void IMEStateChangeHandler::SetInstance(IMEStateChangeHandler* impl) {
  CHECK(!impl_) << "Replacing set IMEStateChangeHandler implementation.";
  impl_ = impl;
}

}  // namespace ui
