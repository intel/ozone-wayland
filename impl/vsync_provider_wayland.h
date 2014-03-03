// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_IMPL_VSYNC_PROVIDER_WAYLAND_H_
#define OZONE_IMPL_VSYNC_PROVIDER_WAYLAND_H_

#include "ui/gfx/vsync_provider.h"

namespace ozonewayland {

class WaylandSyncProvider : public gfx::VSyncProvider {
 public:
  explicit WaylandSyncProvider(unsigned handle);
  virtual ~WaylandSyncProvider();

  virtual void GetVSyncParameters(const UpdateVSyncCallback& callback) OVERRIDE;

 private:
  static void ScheduleFlush();
  unsigned handle_;
  DISALLOW_COPY_AND_ASSIGN(WaylandSyncProvider);
};

}  // namespace ozonewayland

#endif  // OZONE_IMPL_VSYNC_PROVIDER_WAYLAND_H_
