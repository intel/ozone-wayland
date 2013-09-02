// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_INPUT_METHOD_EVENT_FILTER_
#define OZONE_WAYLAND_INPUT_METHOD_EVENT_FILTER_

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "ui/base/ime/input_method_delegate.h"

namespace ui {
class InputMethod;

// An event filter that forwards a KeyEvent to a system IME, and dispatches a
// TranslatedKeyEvent to the root window as needed.
class WaylandInputMethodEventFilter
    : public ui::internal::InputMethodDelegate {
 public:
  WaylandInputMethodEventFilter();
  virtual ~WaylandInputMethodEventFilter();

  InputMethod* GetInputMethod() const { return input_method_.get(); }

 private:
  // Overridden from ui::internal::InputMethodDelegate.
  virtual bool DispatchKeyEventPostIME(const base::NativeEvent& event) OVERRIDE;
  virtual bool DispatchFabricatedKeyEventPostIME(ui::EventType type,
                                                 ui::KeyboardCode key_code,
                                                 int flags) OVERRIDE;

  scoped_ptr<ui::InputMethod> input_method_;

  DISALLOW_COPY_AND_ASSIGN(WaylandInputMethodEventFilter);
};

}  // namespace ui

#endif  // OZONE_WAYLAND_INPUT_METHOD_EVENT_FILTER_
