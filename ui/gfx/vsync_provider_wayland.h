// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_GFX_VSYNC_PROVIDER_WAYLAND_H_
#define OZONE_UI_GFX_VSYNC_PROVIDER_WAYLAND_H_

#include "ui/gfx/vsync_provider.h"

namespace gfx {

class WaylandSyncProvider : public gfx::VSyncProvider {
 public:
  explicit WaylandSyncProvider();
  virtual ~WaylandSyncProvider();

  virtual void GetVSyncParameters(const UpdateVSyncCallback& callback) OVERRIDE;

 private:
  static void ScheduleFlush();
  DISALLOW_COPY_AND_ASSIGN(WaylandSyncProvider);
};

}  // namespace gfx

#endif  // OZONE_UI_GFX_VSYNC_PROVIDER_WAYLAND_H_
