// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_IMPL_VSYNC_PROVIDER_WAYLAND_H_
#define OZONE_IMPL_VSYNC_PROVIDER_WAYLAND_H_

#include "ui/gl/vsync_provider.h"

namespace ozonewayland {

class WaylandWindow;

class WaylandSyncProvider : public gfx::VSyncProvider {
 public:
  WaylandSyncProvider();
  virtual ~WaylandSyncProvider();

  virtual void GetVSyncParameters(const UpdateVSyncCallback& callback) OVERRIDE;

 private:
  static void ScheduleFlush();
  DISALLOW_COPY_AND_ASSIGN(WaylandSyncProvider);
};

}  // namespace ozonewayland

#endif  // OZONE_IMPL_VSYNC_PROVIDER_WAYLAND_H_
