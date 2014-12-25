// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This file contains an implementation of VideoDecoderAccelerator
// that utilizes hardware video decoder present on Intel CPUs.

#ifndef OZONE_MEDIA_VAAPI_TFP_PICTURE_WAYLAND_H_
#define OZONE_MEDIA_VAAPI_TFP_PICTURE_WAYLAND_H_

#include "base/memory/linked_ptr.h"
#include "base/threading/non_thread_safe.h"
#include "ozone/media/vaapi_wrapper.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_surface_egl.h"
#include "ui/gl/scoped_binders.h"

namespace media {

// TFPPicture allocates VAImage and binds them to textures passed
// in PictureBuffers from clients to them. TFPPictures are created as
// a consequence of receiving a set of PictureBuffers from clients and released
// at the end of decode (or when a new set of PictureBuffers is required).
//
// TFPPictures are used for output, contents of VASurfaces passed from decoder
// are put into the associated vaimage memory and upload to client.
class TFPPicture : public base::NonThreadSafe {
 public:
  ~TFPPicture();

  static linked_ptr<TFPPicture> Create(
      const base::Callback<bool(void)>& make_context_current, //NOLINT
      VaapiWrapper* va_wrapper,
      int32 picture_buffer_id,
      uint32 texture_id,
      gfx::Size size);

  int32 picture_buffer_id() {
    return picture_buffer_id_;
  }

  gfx::Size size() {
    return size_;
  }

  bool DownloadFromSurface(const scoped_refptr<VASurface>& va_surface);

 private:
  TFPPicture(const base::Callback<bool(void)>& make_context_current, //NOLINT
             VaapiWrapper* va_wrapper,
             int32 picture_buffer_id,
             uint32 texture_id,
             gfx::Size size);

  bool Initialize();
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

  // Output id for the client.
  int32 picture_buffer_id_;
  uint32 texture_id_;

  gfx::Size size_;
  scoped_ptr<VAImage> va_image_;
  EGLImageKHR egl_image_;
  EGLDisplay egl_display_;
  bool supports_valockBuffer_apis_;

  DISALLOW_COPY_AND_ASSIGN(TFPPicture);
};

}  // namespace media

#endif  // OZONE_MEDIA_VAAPI_TFP_PICTURE_WAYLAND_H_
