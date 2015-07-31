// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_PLATFORM_GPU_PLATFORM_SUPPORT_H_
#define OZONE_PLATFORM_GPU_PLATFORM_SUPPORT_H_

#include <vector>

#include "base/observer_list.h"
#include "base/strings/string16.h"
#include "ozone/ui/events/window_constants.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/ozone/public/gpu_platform_support.h"

namespace IPC {
class Sender;
}

namespace ozonewayland  {
class WaylandDisplay;
}

namespace gfx {
class Point;
}

namespace ui {
class GPUEventDispatcher;

class OzoneGpuPlatformSupport : public GpuPlatformSupport {
 public:
  explicit OzoneGpuPlatformSupport(ozonewayland::WaylandDisplay* display);
  ~OzoneGpuPlatformSupport() override;

  void RegisterHandler(GpuPlatformSupport* handler);
  void UnregisterHandler(GpuPlatformSupport* handler);

  // GpuPlatformSupport:
  void OnChannelEstablished(IPC::Sender* sender) override;
  bool OnMessageReceived(const IPC::Message& message) override;
  void RelinquishGpuResources(const base::Closure& callback) override;
  IPC::MessageFilter* GetMessageFilter() override;

  void OnDisplayStateChanged(unsigned handleid,
                             ui::WidgetState state);
  void OnDisplayTitleChanged(unsigned widget, base::string16 title);
  void OnDisplayCreate(unsigned widget,
                       unsigned parent,
                       unsigned x,
                       unsigned y,
                       ui::WidgetType type);
  void OnDisplayAddRegion(unsigned widget, int left, int top,
                          int right, int bottom);
  void OnDisplaySubRegion(unsigned widget, int left, int top,
                          int right, int bottom);
  void OnDisplaySetCursor(const std::vector<SkBitmap>& bitmaps,
                          const gfx::Point& location);
  void OnDisplayMoveCursor(const gfx::Point& location);
  void OnDisplayImeReset();
  void OnDisplayShowInputPanel();
  void OnDisplayHideInputPanel();

 private:
  IPC::Sender* sender_;
  ozonewayland::WaylandDisplay* display_;
  std::vector<GpuPlatformSupport*> handlers_;  // Not owned.
  DISALLOW_COPY_AND_ASSIGN(OzoneGpuPlatformSupport);
};

}  // namespace ui

#endif  // OZONE_PLATFORM_GPU_PLATFORM_SUPPORT_H_
