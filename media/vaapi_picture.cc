// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/media/vaapi_picture.h"

#include "ozone/media/vaapi_picture_wayland.h"
#include "ozone/media/vaapi_wrapper.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_implementation.h"

namespace content {

// static
linked_ptr<VaapiPicture> VaapiPicture::CreatePicture(
    const scoped_refptr<VaapiWrapper>& vaapi_wrapper,
    const base::Callback<bool(void)> make_context_current,
    int32 picture_buffer_id,
    uint32 texture_id,
    const gfx::Size& size) {
  linked_ptr<VaapiPicture> picture;
  picture.reset(new VaapiPictureWayland(vaapi_wrapper, make_context_current,
                                    picture_buffer_id, texture_id, size));

  if (picture.get() && !picture->Initialize())
    picture.reset();

  return picture;
}

bool VaapiPicture::AllowOverlay() const {
  return false;
}

// static
uint32 VaapiPicture::GetGLTextureTarget() {
  return GL_TEXTURE_2D;
}

}  // namespace content
