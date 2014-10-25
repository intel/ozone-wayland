// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/media/media_ozone_platform_wayland.h"

#include "media/video/video_decode_accelerator.h"
#include "media/ozone/media_ozone_platform.h"
#include "ozone/media/vaapi_video_decode_accelerator.h"

namespace media {

namespace {

class MediaOzonePlatformWayland : public MediaOzonePlatform {
 public:
  MediaOzonePlatformWayland() {}

  virtual ~MediaOzonePlatformWayland() {}

  // MediaOzonePlatform:
  virtual media::VideoDecodeAccelerator* CreateVideoDecodeAccelerator(
      const base::Callback<bool(void)>& make_context_current) override {
    return new VaapiVideoDecodeAccelerator(make_context_current);
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(MediaOzonePlatformWayland);
};

}  // namespace

MediaOzonePlatform* CreateMediaOzonePlatformWayland() { return new MediaOzonePlatformWayland; }

}  // namespace media
