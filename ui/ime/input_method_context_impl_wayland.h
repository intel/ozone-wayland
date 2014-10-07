// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_IME_INPUT_METHOD_CONTEXT_IMPL_WAYLAND_H_
#define OZONE_UI_IME_INPUT_METHOD_CONTEXT_IMPL_WAYLAND_H_

#include "ozone/platform/ozone_export_wayland.h"
#include "ozone/ui/events/ime_change_observer.h"
#include "ui/base/ime/linux/linux_input_method_context.h"
#include "ui/gfx/rect.h"

namespace ui {

// An implementation of LinuxInputMethodContext for IME support on Ozone
// platform using Wayland.
class OZONE_WAYLAND_EXPORT InputMethodContextImplWayland
    : public LinuxInputMethodContext,
      public IMEChangeObserver {
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

  // Overriden from ui::IMEChangeObserver
  virtual void OnPreeditChanged(unsigned handle,
                                const std::string& text,
                                const std::string& commit) OVERRIDE;
  virtual void OnCommit(unsigned handle, const std::string& text) OVERRIDE;

 private:
  // Must not be NULL.
  LinuxInputMethodContextDelegate* delegate_;
  DISALLOW_COPY_AND_ASSIGN(InputMethodContextImplWayland);
};

}  // namespace ui

#endif  // OZONE_UI_IME_INPUT_METHOD_CONTEXT_IMPL_WAYLAND_H_
