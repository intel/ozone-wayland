// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_WEBUI_INPUT_METHOD_CONTEXT_IMPL_WAYLAND_H_
#define OZONE_UI_WEBUI_INPUT_METHOD_CONTEXT_IMPL_WAYLAND_H_

#include <string>

#include "base/memory/weak_ptr.h"
#include "ozone/platform/ozone_export_wayland.h"
#include "ui/base/ime/linux/linux_input_method_context.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/ozone/public/gpu_platform_support_host.h"

namespace ui {

class OzoneGpuPlatformSupportHost;

// An implementation of LinuxInputMethodContext for IME support on Ozone
// platform using Wayland.
class OZONE_WAYLAND_EXPORT InputMethodContextImplWayland
  : public LinuxInputMethodContext,
    public GpuPlatformSupportHost {
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

 private:
  // GpuPlatformSupportHost
  void OnChannelEstablished(
      int host_id,
      scoped_refptr<base::SingleThreadTaskRunner> send_runner,
      const base::Callback<void(IPC::Message*)>& send_callback) override;
  void OnChannelDestroyed(int host_id) override;
  bool OnMessageReceived(const IPC::Message&) override;
  void OnPreeditChanged(unsigned handle,
                        const std::string& text,
                        const std::string& commit);
  void OnCommit(unsigned handle, const std::string& text);
  void ShowInputPanel();
  void HideInputPanel();
  void Commit(unsigned handle, const std::string& text);
  void PreeditChanged(unsigned handle,
                      const std::string& text,
                      const std::string& commit);
  void PreeditEnd();
  void PreeditStart();

  // Must not be NULL.
  LinuxInputMethodContextDelegate* delegate_;
  OzoneGpuPlatformSupportHost* sender_;  // Not owned.
  // Support weak pointers for attach & detach callbacks.
  base::WeakPtrFactory<InputMethodContextImplWayland> weak_ptr_factory_;
  DISALLOW_COPY_AND_ASSIGN(InputMethodContextImplWayland);
};

}  // namespace ui

#endif  //  OZONE_UI_WEBUI_INPUT_METHOD_CONTEXT_IMPL_WAYLAND_H_
