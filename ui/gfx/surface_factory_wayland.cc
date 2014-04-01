// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/gfx/surface_factory_wayland.h"

#include <EGL/egl.h>
#include <string>
#include "base/files/file_path.h"
#include "base/native_library.h"
#include "ozone/ui/gfx/ozone_display.h"
#include "ozone/ui/gfx/vsync_provider_wayland.h"

namespace gfx {

SurfaceFactoryWayland::SurfaceFactoryWayland()
    : last_realized_widget_handle_(0) {
}

SurfaceFactoryWayland::~SurfaceFactoryWayland() {
}

gfx::SurfaceFactoryOzone::HardwareState
SurfaceFactoryWayland::InitializeHardware() {
  return OzoneDisplay::GetInstance()->InitializeHardware();
}

intptr_t SurfaceFactoryWayland::GetNativeDisplay() {
  return OzoneDisplay::GetInstance()->GetNativeDisplay();
}

void SurfaceFactoryWayland::ShutdownHardware() {
  OzoneDisplay::GetInstance()->ShutdownHardware();
}

gfx::AcceleratedWidget SurfaceFactoryWayland::GetAcceleratedWidget() {
  return OzoneDisplay::GetInstance()->GetAcceleratedWidget();
}

gfx::AcceleratedWidget SurfaceFactoryWayland::RealizeAcceleratedWidget(
    gfx::AcceleratedWidget w) {
  DCHECK(OzoneDisplay::GetInstance());
  last_realized_widget_handle_ = w;
  return OzoneDisplay::GetInstance()->RealizeAcceleratedWidget(w);
}

bool SurfaceFactoryWayland::LoadEGLGLES2Bindings(
    gfx::SurfaceFactoryOzone::AddGLLibraryCallback add_gl_library,
    gfx::SurfaceFactoryOzone::SetGLGetProcAddressProcCallback setprocaddress) {
  // The variable EGL_PLATFORM specifies native platform to be used by the
  // drivers (atleast on Mesa). When the variable is not set, Mesa uses the
  // first platform listed in --with-egl-platforms during compilation. Thus, we
  // ensure here that wayland is set as the native platform. However, we don't
  // override the EGL_PLATFORM value in case it has already been set.
  setenv("EGL_PLATFORM", "wayland", 0);
  base::NativeLibraryLoadError error;
  base::NativeLibrary gles_library = base::LoadNativeLibrary(
    base::FilePath("libGLESv2.so.2"), &error);

  if (!gles_library) {
    LOG(WARNING) << "Failed to load GLES library: " << error.ToString();
    return false;
  }

  base::NativeLibrary egl_library = base::LoadNativeLibrary(
    base::FilePath("libEGL.so.1"), &error);

  if (!egl_library) {
    LOG(WARNING) << "Failed to load EGL library: " << error.ToString();
    base::UnloadNativeLibrary(gles_library);
    return false;
  }

  GLGetProcAddressProc get_proc_address =
      reinterpret_cast<GLGetProcAddressProc>(
          base::GetFunctionPointerFromNativeLibrary(
              egl_library, "eglGetProcAddress"));

  if (!get_proc_address) {
    LOG(ERROR) << "eglGetProcAddress not found.";
    base::UnloadNativeLibrary(egl_library);
    base::UnloadNativeLibrary(gles_library);
    return false;
  }

  setprocaddress.Run(get_proc_address);
  add_gl_library.Run(egl_library);
  add_gl_library.Run(gles_library);
  return true;
}

bool SurfaceFactoryWayland::AttemptToResizeAcceleratedWidget(
         gfx::AcceleratedWidget w, const gfx::Rect& bounds) {
  return OzoneDisplay::GetInstance()->AttemptToResizeAcceleratedWidget(
      w, bounds);
}

scoped_ptr<gfx::VSyncProvider>
SurfaceFactoryWayland::CreateVSyncProvider(gfx::AcceleratedWidget w) {
  DCHECK(last_realized_widget_handle_);
  // This is based on the fact that we realize accelerated widget and create
  // its vsync provider immediately (right after widget is realized). This
  // saves us going through list of realized widgets and finding the right one.
  unsigned handle = last_realized_widget_handle_;
  last_realized_widget_handle_ = 0;
  return scoped_ptr<gfx::VSyncProvider>(new WaylandSyncProvider(handle));
}

bool SurfaceFactoryWayland::SchedulePageFlip(gfx::AcceleratedWidget w) {
  return true;
}

const int32*
SurfaceFactoryWayland::GetEGLSurfaceProperties(const int32* desired_list) {
  static const EGLint kConfigAttribs[] = {
    EGL_BUFFER_SIZE, 32,
    EGL_ALPHA_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_RED_SIZE, 8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_NONE
  };

  return kConfigAttribs;
}

}  // namespace gfx
