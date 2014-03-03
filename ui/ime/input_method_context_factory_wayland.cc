// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/ime/input_method_context_factory_wayland.h"

#include "ozone/ui/ime/input_method_context_impl_wayland.h"

namespace ui {

InputMethodContextFactoryWayland::InputMethodContextFactoryWayland()
    : ui::InputMethodContextFactoryOzone() {
}

InputMethodContextFactoryWayland::~InputMethodContextFactoryWayland() {
}

scoped_ptr<ui::LinuxInputMethodContext>
InputMethodContextFactoryWayland::CreateInputMethodContext(
      ui::LinuxInputMethodContextDelegate* delegate) const {
  return scoped_ptr<ui::LinuxInputMethodContext>(
             new InputMethodContextImplWayland(delegate));
}

}  // namespace ui
