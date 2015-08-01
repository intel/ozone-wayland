// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_WEBUI_INPUT_METHOD_CONTEXT_IMPL_WAYLAND_H_
#define OZONE_UI_WEBUI_INPUT_METHOD_CONTEXT_IMPL_WAYLAND_H_

#include <string>

#include "ozone/platform/ozone_export_wayland.h"
#include "ozone/ui/events/ime_change_observer.h"
#include "ui/base/ime/linux/linux_input_method_context.h"
#include "ui/gfx/geometry/rect.h"

namespace ui {

class OzoneGpuPlatformSupportHost;

// An implementation of LinuxInputMethodContext for IME support on Ozone
// platform using Wayland.
class OZONE_WAYLAND_EXPORT InputMethodContextImplWayland
  : public LinuxInputMethodContext,
    public IMEChangeObserver {
 public:
  InputMethodContextImplWayland(
      ui::LinuxInputMethodContextDelegate* delegate,
      OzoneGpuPlatformSupportHost* sender);
  ~InputMethodContextImplWayland() override;

  // overriden from ui::LinuxInputMethodContext
  bool DispatchKeyEvent(const ui::KeyEvent& key_event) override;
  void Reset() override;
  void Focus() override;
  void Blur() override;
  void SetCursorLocation(const gfx::Rect&) override;


  // overriden from ui::IMEChangeObserver
  void OnPreeditChanged(unsigned handle,
                        const std::string& text,
                        const std::string& commit) override;
  void OnCommit(unsigned handle, const std::string& text) override;

 private:
  void ShowInputPanel();
  void HideInputPanel();
  // Must not be NULL.
  LinuxInputMethodContextDelegate* delegate_;
  OzoneGpuPlatformSupportHost* sender_;  // Not owned.
  DISALLOW_COPY_AND_ASSIGN(InputMethodContextImplWayland);
};

}  // namespace ui

#endif  //  OZONE_UI_WEBUI_INPUT_METHOD_CONTEXT_IMPL_WAYLAND_H_
