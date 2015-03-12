// Copyright (c) 2014 The Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/media/vaapi_picture_wayland.h"

#include <string>

namespace content {

VaapiPictureWayland::VaapiPictureWayland(
    VaapiWrapper* vaapi_wrapper,
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
  supports_vaAcquireBufferHandle_apis_ =
      va_wrapper_->SupportsVaAcquireBufferHandleApis();
  std::string query =
      supports_vaAcquireBufferHandle_apis_ ? "supports" : "doesn't support";
  LOG(INFO) << "VAAPI " << query << " vaAcquireBufferHandle apis";
}

bool VaapiPictureWayland::Initialize() {
  DCHECK(CalledOnValidThread());
  if (!make_context_current_.Run())
    return false;

  if (!va_wrapper_->CreateRGBImage(size(), va_image_.get())) {
    DVLOG(1) << "Failed to create VAImage";
    return false;
  }

  return true;
}

VaapiPictureWayland::~VaapiPictureWayland() {
  DCHECK(CalledOnValidThread());

  if (supports_vaAcquireBufferHandle_apis_ && gl_image_)
    gl_image_->Destroy(true);

  if (va_wrapper_) {
    va_wrapper_->DestroyImage(va_image_.get());
  }
}

bool VaapiPictureWayland::Upload(VASurfaceID surface) {
  DCHECK(CalledOnValidThread());

  if (!make_context_current_.Run())
    return false;

  if (!va_wrapper_->PutSurfaceIntoImage(surface, va_image_.get())) {
    DVLOG(1) << "Failed to put va surface to image";
    return false;
  }

  void* buffer = NULL;
  if (!va_wrapper_->MapImage(va_image_.get(), &buffer)) {
    DVLOG(1) << "Failed to map VAImage";
    return false;
  }

  gfx::ScopedTextureBinder texture_binder(GL_TEXTURE_2D, texture_id());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  // See bug https://crosswalk-project.org/jira/browse/XWALK-2265.
  // The following small piece of code is a workaround for the current VDA
  // texture output implementation. It can be removed when zero buffer copy
  // is implemented.
  int picture_width = size().width();
  int picture_height = size().height();
  unsigned int al = 4 * picture_width;
  if (al != va_image_->pitches[0]) {
    // Not aligned phenomenon occurs only in special size video in None-X11.
    // So re-check RGBA data alignment and realign filled video frame in need.
    unsigned char* bhandle = static_cast<unsigned char*>(buffer);
    for (int i = 0; i < picture_height; i++) {
      memcpy(bhandle + (i * al), bhandle + (i * (va_image_->pitches[0])), al);
    }
  }

  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_BGRA,
               picture_width,
               picture_height,
               0,
               GL_BGRA,
               GL_UNSIGNED_BYTE,
               buffer);

  va_wrapper_->UnmapImage(va_image_.get());

  return true;
}

bool VaapiPictureWayland::CreateEGLImage(VAImage* va_image) {
  DCHECK(va_image);

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
  gl_image_ = new gfx::GLImageEGL(size());
  if (!gl_image_->Initialize(EGL_DRM_BUFFER_MESA,
                             (EGLClientBuffer) buffer_info.handle,
                             attribs)) {
    LOG(ERROR) << "Failed to create a GLImageEGL for a Va Pixmap.";
    return false;
  }

  if (va_wrapper_) {
    va_wrapper_->ReleaseBufferHandle(va_image->buf);
  }

  return true;
}

bool VaapiPictureWayland::DownloadFromSurface(
    const scoped_refptr<VASurface>& va_surface) {
  if (supports_vaAcquireBufferHandle_apis_) {
    if (!va_wrapper_->PutSurfaceIntoImage(va_surface->id(), va_image_.get()))
      return false;

    DCHECK(CalledOnValidThread());

    if (!make_context_current_.Run())
      return false;

    if (gl_image_)
      gl_image_->Destroy(true);

    if (!CreateEGLImage(va_image_.get()))
       return false;

    gfx::ScopedTextureBinder texture_binder(GL_TEXTURE_2D, texture_id());
    if (!gl_image_->BindTexImage(GL_TEXTURE_2D)) {
      LOG(ERROR) << "Failed to bind texture to GLImage";
      return false;
    }
  } else {
    if (!Upload(va_surface->id()))
      return false;
  }

  return true;
}

scoped_refptr<gfx::GLImage> VaapiPictureWayland::GetImageToBind() {
  return gl_image_;
}

}  // namespace content
