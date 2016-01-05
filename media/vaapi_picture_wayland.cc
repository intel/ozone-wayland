// Copyright (c) 2014 The Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/media/vaapi_picture_wayland.h"

#include <string>

namespace content {

VaapiPictureWayland::VaapiPictureWayland(
    const scoped_refptr<VaapiWrapper>& vaapi_wrapper,
    const base::Callback<bool(void)> make_context_current,
    int32 picture_buffer_id,
    uint32 texture_id,
    const gfx::Size& size)
    : VaapiPicture(picture_buffer_id, texture_id, size),
      make_context_current_(make_context_current),
      va_wrapper_(vaapi_wrapper),
      va_image_(new VAImage()) {
  DCHECK(!make_context_current_.is_null());
  DCHECK(va_image_);
}

bool VaapiPictureWayland::Initialize() {
  DCHECK(CalledOnValidThread());
  if (!make_context_current_.Run())
    return false;

  if (!va_wrapper_->CreateRGBImage(size(), va_image_.get())) {
    DVLOG(1) << "Failed to create VAImage";
    return false;
  }

  if (!CreateEGLImage(va_image_.get()))
     return false;

  gfx::ScopedTextureBinder texture_binder(GL_TEXTURE_2D, texture_id());
  if (!gl_image_->BindTexImage(GL_TEXTURE_2D)) {
    LOG(ERROR) << "Failed to bind texture to GLImage";
    return false;
  }

  return true;
}

VaapiPictureWayland::~VaapiPictureWayland() {
  DCHECK(CalledOnValidThread());

  if (gl_image_ && make_context_current_.Run()) {
    gl_image_->ReleaseTexImage(GL_TEXTURE_2D);
    gl_image_->Destroy(true);

    DCHECK_EQ(glGetError(), static_cast<GLenum>(GL_NO_ERROR));
  }

  if (va_wrapper_)
    va_wrapper_->DestroyImage(va_image_.get());
}

bool VaapiPictureWayland::CreateEGLImage(VAImage* va_image) {
  DCHECK(va_image);
  DCHECK(va_wrapper_);

  VABufferInfo buffer_info;
  if (!va_wrapper_->AcquireBufferHandle(va_image->buf, &buffer_info)) {
    DVLOG(1) << "Failed to acquire buffer handle";
    return false;
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

  // Create an EGLImage out of the same buffer.
  gl_image_ = new gl::GLImageEGL(size());
  if (!gl_image_->Initialize(EGL_DRM_BUFFER_MESA,
                             (EGLClientBuffer) buffer_info.handle,
                             attribs)) {
    LOG(ERROR) << "Failed to create a GLImageEGL for a Va Pixmap.";
    return false;
  }

  va_wrapper_->ReleaseBufferHandle(va_image->buf);

  return true;
}

bool VaapiPictureWayland::DownloadFromSurface(
    const scoped_refptr<VASurface>& va_surface) {
    DCHECK(CalledOnValidThread());
    if (!va_wrapper_->PutSurfaceIntoImage(va_surface->id(), va_image_.get()))
      return false;

  return true;
}

scoped_refptr<gl::GLImage> VaapiPictureWayland::GetImageToBind() {
  return gl_image_;
}

}  // namespace content
