// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/egl/wayland_pixmap.h"

#include <gbm.h>

#include "base/logging.h"

namespace ozonewayland {

namespace {

int GetGbmFormatFromBufferFormat(ui::SurfaceFactoryOzone::BufferFormat fmt) {
  switch (fmt) {
    case ui::SurfaceFactoryOzone::RGBA_8888:
      return GBM_BO_FORMAT_ARGB8888;
    case ui::SurfaceFactoryOzone::RGBX_8888:
      return GBM_BO_FORMAT_XRGB8888;
    default:
      NOTREACHED();
      return 0;
  }
}

}  // namespace

WaylandPixmap::WaylandPixmap()
    : bo_(NULL), dma_buf_(-1)  {
}

bool WaylandPixmap::Initialize(gbm_device* device,
                               ui::SurfaceFactoryOzone::BufferFormat format,
                               const gfx::Size& size) {
  unsigned flags = GBM_BO_USE_RENDERING;
  bo_ = gbm_bo_create(device,
                      size.width(),
                      size.height(),
                      GetGbmFormatFromBufferFormat(format),
                      flags);
  if (!bo_) {
    LOG(ERROR) << "Failed to create GBM buffer object.";
    return false;
  }

  dma_buf_ = gbm_bo_get_fd(bo_);
  if (dma_buf_ < 0) {
    LOG(ERROR) << "Failed to export buffer to dma_buf.";
    return false;
  }

  return true;
}

WaylandPixmap::~WaylandPixmap() {
  if (bo_)
    gbm_bo_destroy(bo_);

  if (dma_buf_ > 0)
    close(dma_buf_);
}

void* WaylandPixmap::GetEGLClientBuffer() {
  return bo_;
}

int WaylandPixmap::GetDmaBufFd() {
  return dma_buf_;
}

int WaylandPixmap::GetDmaBufPitch() {
  return gbm_bo_get_stride(bo_);
}

}  // namespace ozonewayland
