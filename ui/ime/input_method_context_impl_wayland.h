// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_IME_INPUT_METHOD_CONTEXT_IMPL_WAYLAND_H_
#define OZONE_UI_IME_INPUT_METHOD_CONTEXT_IMPL_WAYLAND_H_

#include "ozone/platform/ozone_export_wayland.h"
#include "ui/base/ime/linux/linux_input_method_context.h"
#include "ui/gfx/rect.h"

namespace ozonewayland {

// An implementation of LinuxInputMethodContext for IME support on Ozone
// platform using Wayland.
class OZONE_WAYLAND_EXPORT InputMethodContextImplWayland
    : public ui::LinuxInputMethodContext {
 public:
  explicit InputMethodContextImplWayland(
      ui::LinuxInputMethodContextDelegate* delegate);
  virtual ~InputMethodContextImplWayland();

  // Overriden from ui::LinuxInputMethodContext
  virtual bool DispatchKeyEvent(const ui::KeyEvent& key_event) OVERRIDE;
  virtual void Reset() OVERRIDE;
  virtual void OnTextInputTypeChanged(ui::TextInputType text_input_type)
      OVERRIDE;
  virtual void OnCaretBoundsChanged(const gfx::Rect& caret_bounds) OVERRIDE;

 private:
  // Must not be NULL.
  ui::LinuxInputMethodContextDelegate* delegate_;
  DISALLOW_COPY_AND_ASSIGN(InputMethodContextImplWayland);
};

}  // namespace ozonewayland

#endif  // OZONE_UI_IME_INPUT_METHOD_CONTEXT_IMPL_WAYLAND_H_
