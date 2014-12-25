// Copyright (c) 2014 The Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/media/vaapi_tfp_picture_wayland.h"

#include <string>

namespace media {

TFPPicture::TFPPicture(
    const base::Callback<bool(void)>& make_context_current, //NOLINT
    VaapiWrapper* va_wrapper,
    int32 picture_buffer_id,
    uint32 texture_id,
    gfx::Size size)
    : make_context_current_(make_context_current),
      va_wrapper_(va_wrapper),
      picture_buffer_id_(picture_buffer_id),
      texture_id_(texture_id),
      size_(size),
      va_image_(new VAImage()),
      egl_display_(gfx::GLSurfaceEGL::GetHardwareDisplay()) {
  DCHECK(!make_context_current_.is_null());
  DCHECK(va_image_);
  supports_valockBuffer_apis_ = va_wrapper_->SupportsVaLockBufferApis();
  std::string query =
      supports_valockBuffer_apis_ ? "supports" : "doesn't support";
  LOG(INFO) << "VAAPI " << query << " vaLockBuffer apis";
  fprintf(stderr, "TFPPicture created..................... \n");
}

linked_ptr<TFPPicture>
TFPPicture::Create(
    const base::Callback<bool(void)>& make_context_current, //NOLINT
    VaapiWrapper* va_wrapper,
    int32 picture_buffer_id,
    uint32 texture_id,
    gfx::Size size) {
  linked_ptr<TFPPicture> tfp_picture(
      new TFPPicture(make_context_current, va_wrapper,
                     picture_buffer_id, texture_id, size));

  if (!tfp_picture->Initialize())
    tfp_picture.reset();

  return tfp_picture;
}

bool TFPPicture::Initialize() {
  DCHECK(CalledOnValidThread());
  if (!make_context_current_.Run())
    return false;

  if (!va_wrapper_->CreateRGBImage(size_, va_image_.get())) {
    DVLOG(1) << "Failed to create VAImage";
    return false;
  }

  return true;
}

TFPPicture::~TFPPicture() {
  DCHECK(CalledOnValidThread());

  if (supports_valockBuffer_apis_ && egl_image_ != EGL_NO_IMAGE_KHR)
    DestroyEGLImage(egl_display_, egl_image_);

  if (va_wrapper_) {
    va_wrapper_->DestroyImage(va_image_.get());
  }
}

bool TFPPicture::UpdateEGLImage(
    VASurfaceID surface) {
  DCHECK(CalledOnValidThread());

  if (!make_context_current_.Run())
    return false;

  if (egl_image_ != EGL_NO_IMAGE_KHR)
    DestroyEGLImage(egl_display_, egl_image_);

  egl_image_ = CreateEGLImage(egl_display_, surface, va_image_.get());
  if (egl_image_ == EGL_NO_IMAGE_KHR) {
    DVLOG(1) << "Failed to create EGL image";
    return false;
  }

  return true;
}

bool TFPPicture::Upload(VASurfaceID surface) {
  DCHECK(CalledOnValidThread());

  if (!make_context_current_.Run())
    return false;
fprintf(stderr, "using slow upload..................... \n");
  if (!va_wrapper_->PutSurfaceIntoImage(surface, va_image_.get())) {
    DVLOG(1) << "Failed to put va surface to image";
    return false;
  }

  void* buffer = NULL;
  if (!va_wrapper_->MapImage(va_image_.get(), &buffer)) {
    DVLOG(1) << "Failed to map VAImage";
    return false;
  }

  gfx::ScopedTextureBinder texture_binder(GL_TEXTURE_2D, texture_id_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  // See bug https://crosswalk-project.org/jira/browse/XWALK-2265.
  // The following small piece of code is a workaround for the current VDA
  // texture output implementation. It can be removed when zero buffer copy
  // is implemented.
  unsigned int al = 4 * size_.width();
  if (al != va_image_->pitches[0]) {
    // Not aligned phenomenon occurs only in special size video in None-X11.
    // So re-check RGBA data alignment and realign filled video frame in need.
    unsigned char* bhandle = static_cast<unsigned char*>(buffer);
    for (int i = 0; i < size_.height(); i++) {
      memcpy(bhandle + (i * al), bhandle + (i * (va_image_->pitches[0])), al);
    }
  }

  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_BGRA,
               size_.width(),
               size_.height(),
               0,
               GL_BGRA,
               GL_UNSIGNED_BYTE,
               buffer);

  va_wrapper_->UnmapImage(va_image_.get());

  return true;
}

bool TFPPicture::Bind() {
  DCHECK(CalledOnValidThread());
  if (!make_context_current_.Run())
    return false;

  gfx::ScopedTextureBinder texture_binder(GL_TEXTURE_2D, texture_id_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, egl_image_);
  return true;
}

EGLImageKHR TFPPicture::CreateEGLImage(
    EGLDisplay egl_display, VASurfaceID va_surface, VAImage* va_image) {
  DCHECK(CalledOnValidThread());
  DCHECK(va_image);

  VABufferInfo buffer_info;
  if (!va_wrapper_->LockBuffer(va_surface, va_image->buf, &buffer_info)) {
    DVLOG(1) << "Failed to lock Buffer";
    return EGL_NO_IMAGE_KHR;
  }

  EGLint attribs[] = {
      EGL_WIDTH, 0,
      EGL_HEIGHT, 0,
      EGL_DRM_BUFFER_STRIDE_MESA, 0,
      EGL_DRM_BUFFER_FORMAT_MESA,
      EGL_DRM_BUFFER_FORMAT_ARGB32_MESA,
      EGL_DRM_BUFFER_USE_MESA,
      EGL_DRM_BUFFER_USE_SHARE_MESA,
      EGL_NONE };
  attribs[1] = va_image->width;
  attribs[3] = va_image->height;
  attribs[5] = va_image->pitches[0] / 4;

  EGLImageKHR egl_image =  eglCreateImageKHR(
       egl_display,
       EGL_NO_CONTEXT,
       EGL_DRM_BUFFER_MESA,
       (EGLClientBuffer) buffer_info.handle,
       attribs);

  if (va_wrapper_) {
    va_wrapper_->UnlockBuffer(va_surface, va_image->buf, &buffer_info);
  }

  return egl_image;
}

bool TFPPicture::DestroyEGLImage(
    EGLDisplay egl_display, EGLImageKHR egl_image) {
  return eglDestroyImageKHR(egl_display, egl_image);
}

bool TFPPicture::DownloadFromSurface(
    const scoped_refptr<VASurface>& va_surface) {
  if (supports_valockBuffer_apis_) {
    if (!va_wrapper_->PutSurfaceIntoImage(va_surface->id(), va_image_.get()))
      return false;

    if (!UpdateEGLImage(va_surface->id()))
      return false;

    if (!Bind())
      return false;
  } else {
    if (!Upload(va_surface->id()))
      return false;
  }

  return true;
}

}  // namespace media
