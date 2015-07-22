// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_PUBLIC_OZONE_GPU_PLATFORM_SUPPORT_H_
#define OZONE_UI_PUBLIC_OZONE_GPU_PLATFORM_SUPPORT_H_

#include <vector>

#include "base/observer_list.h"
#include "base/strings/string16.h"
#include "ozone/ui/events/window_constants.h"
#include "ui/ozone/public/gpu_platform_support.h"

namespace IPC {
class Sender;
}

namespace ui {
class RemoteEventDispatcher;

class OzoneGpuPlatformSupport : public GpuPlatformSupport {
 public:
  OzoneGpuPlatformSupport();
  ~OzoneGpuPlatformSupport() override;

  void RegisterHandler(GpuPlatformSupport* handler);
  void UnregisterHandler(GpuPlatformSupport* handler);

  // GpuPlatformSupport:
  void OnChannelEstablished(IPC::Sender* sender) override;
  bool OnMessageReceived(const IPC::Message& message) override;
  void RelinquishGpuResources(const base::Closure& callback) override;
  IPC::MessageFilter* GetMessageFilter() override;

  void OnWidgetStateChanged(unsigned handleid,
                            ui::WidgetState state);
  void OnWidgetTitleChanged(unsigned widget, base::string16 title);
  void OnWidgetCursorChanged(int cursor_type);
  void OnWidgetCreate(unsigned widget,
                      unsigned parent,
                      unsigned x,
                      unsigned y,
                      ui::WidgetType type);
  void OnWidgetImeReset();
  void OnWidgetShowInputPanel();
  void OnWidgetHideInputPanel();

 private:
  IPC::Sender* sender_;
  std::vector<GpuPlatformSupport*> handlers_;  // Not owned.
  DISALLOW_COPY_AND_ASSIGN(OzoneGpuPlatformSupport);
};

}  // namespace ui

#endif  // OZONE_UI_PUBLIC_OZONE_GPU_PLATFORM_SUPPORT_H_
