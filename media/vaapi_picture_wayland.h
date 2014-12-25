// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This file contains an implementation of VideoDecoderAccelerator
// that utilizes hardware video decoder present on Intel CPUs.

#ifndef OZONE_MEDIA_VAAPI_PICTURE_WAYLAND_H_
#define OZONE_MEDIA_VAAPI_PICTURE_WAYLAND_H_

#include "base/memory/linked_ptr.h"
#include "base/threading/non_thread_safe.h"
#include "ozone/media/vaapi_picture.h"
#include "ozone/media/vaapi_wrapper.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_surface_egl.h"
#include "ui/gl/scoped_binders.h"

namespace media {

class VaapiPictureWayland : public VaapiPicture {
 public:
  VaapiPictureWayland(VaapiWrapper* vaapi_wrapper,
                      const base::Callback<bool(void)> make_context_current,
                      int32 picture_buffer_id,
                      uint32 texture_id,
                      const gfx::Size& size);
  ~VaapiPictureWayland();

  bool Initialize() override;
  bool DownloadFromSurface(const scoped_refptr<VASurface>& va_surface) override;

 private:
  // Upload vaimage data to texture. Needs to be called every frame.
  bool Upload(VASurfaceID id);

  // Bind EGL image to texture. Needs to be called every frame.
  bool Bind();
  bool UpdateEGLImage(VASurfaceID id);

  EGLImageKHR CreateEGLImage(
      EGLDisplay egl_display, VASurfaceID surface, VAImage* va_image);
  bool DestroyEGLImage(EGLDisplay egl_display, EGLImageKHR egl_image);

  base::Callback<bool(void)> make_context_current_; //NOLINT

  VaapiWrapper* va_wrapper_;

  scoped_ptr<VAImage> va_image_;
  EGLImageKHR egl_image_;
  EGLDisplay egl_display_;
  bool supports_valockBuffer_apis_;

  DISALLOW_COPY_AND_ASSIGN(VaapiPictureWayland);
};

}  // namespace media

#endif  // OZONE_MEDIA_VAAPI_TFP_PICTURE_WAYLAND_H_
