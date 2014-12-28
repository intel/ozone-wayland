// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_PIXMAP_
#define OZONE_WAYLAND_PIXMAP_

#include "base/macros.h"
#include "ui/gfx/geometry/size.h"
#include "ui/ozone/public/native_pixmap.h"
#include "ui/ozone/public/surface_factory_ozone.h"

struct gbm_bo;
struct gbm_device;

namespace ozonewayland {

class WaylandPixmap : public ui::NativePixmap {
 public:
  WaylandPixmap();
  bool Initialize(gbm_device* device,
                  ui::SurfaceFactoryOzone::BufferFormat format,
                  const gfx::Size& size);

  // NativePixmap:
  void* GetEGLClientBuffer() override;
  int GetDmaBufFd() override;
  int GetDmaBufPitch() override;

 private:
  ~WaylandPixmap() override;

  gbm_bo* bo_;
  int dma_buf_;

  DISALLOW_COPY_AND_ASSIGN(WaylandPixmap);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_PIXMAP_
