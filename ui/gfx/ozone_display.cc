// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/gfx/ozone_display.h"

#include "base/logging.h"
#include "ozone/ui/events/window_state_change_handler.h"
#include "ui/ozone/public/surface_ozone_egl.h"

namespace ui {

// static
OzoneDisplay* OzoneDisplay::instance_ = NULL;

OzoneDisplay::OzoneDisplay() {
}

OzoneDisplay::~OzoneDisplay() {
}

OzoneDisplay* OzoneDisplay::GetInstance() {
  CHECK(instance_) << "No OzoneDisplay implementation set.";
  return instance_;
}

void OzoneDisplay::SetInstance(OzoneDisplay* instance) {
  CHECK(!instance_) << "Replacing set OzoneDisplay implementation.";
  instance_ = instance;
}

void OzoneDisplay::LookAheadOutputGeometry() {
}

intptr_t OzoneDisplay::GetNativeDisplay() {
  return (intptr_t)NULL;
}

gfx::AcceleratedWidget OzoneDisplay::GetAcceleratedWidget() {
  static int opaque_handle = 0;
  opaque_handle++;
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(opaque_handle,
                                                              ui::CREATE,
                                                              0,
                                                              0);

  return (gfx::AcceleratedWidget)opaque_handle;
}

scoped_ptr<SurfaceOzoneEGL> OzoneDisplay::CreateEGLSurfaceForWidget(
    gfx::AcceleratedWidget w) {
  return scoped_ptr<SurfaceOzoneEGL>();
}

bool OzoneDisplay::LoadEGLGLES2Bindings(
    SurfaceFactoryOzone::AddGLLibraryCallback add_gl_library,
    SurfaceFactoryOzone::SetGLGetProcAddressProcCallback setprocaddress) {
  return false;
}

const int32*
OzoneDisplay::GetEGLSurfaceProperties(const int32* desired_list) {
  return desired_list;
}

}  // namespace ui
