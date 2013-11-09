// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/input/input_method_event_filter.h"

#include "ui/base/ime/input_method.h"
#include "ui/base/ime/input_method_factory.h"

namespace ozonewayland {

////////////////////////////////////////////////////////////////////////////////
// InputMethodEventFilter, public:

WaylandInputMethodEventFilter::WaylandInputMethodEventFilter()
    : input_method_(ui::CreateInputMethod(this, 0)) {
  // TODO(yusukes): Check if the root window is currently focused and pass the
  // result to Init().
  input_method_->Init(true);
}

WaylandInputMethodEventFilter::~WaylandInputMethodEventFilter()
{
}

////////////////////////////////////////////////////////////////////////////////
// InputMethodEventFilter, ui::InputMethodDelegate implementation:

bool WaylandInputMethodEventFilter::DispatchKeyEventPostIME(
    const base::NativeEvent& event)
{
}

bool WaylandInputMethodEventFilter::DispatchFabricatedKeyEventPostIME(
    ui::EventType type,
    ui::KeyboardCode key_code,
    int flags)
{
}

}  // namespace ozonewayland
