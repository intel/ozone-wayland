// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/desktop_screen_wayland.h"
#include "ozone/surface_factory_wayland.h"
#include "ozone/event_factory_wayland.h"

#include "base/base_paths.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/native_library.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/string_util.h"
#include "ozone/wayland_display.h"
#include "ozone/wayland_window.h"
#include "ozone/wayland_screen.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_egl_api_implementation.h"
#include "ui/gl/gl_gl_api_implementation.h"
#include "ui/gl/gl_implementation.h"
#include "ui/gl/gl_switches.h"

namespace gfx {
namespace {

// Load a library, printing an error message on failure.
base::NativeLibrary LoadLibrary(const base::FilePath& filename) {
  std::string error;
  base::NativeLibrary library = base::LoadNativeLibrary(filename,
                                                        &error);
  if (!library) {
    DVLOG(1) << "Failed to load " << filename.MaybeAsASCII() << ": " << error;
    return NULL;
  }
  return library;
}

base::NativeLibrary LoadLibrary(const char* filename) {
  return LoadLibrary(base::FilePath(filename));
}

}  // namespace

bool InitializeGLBindings() {
  base::NativeLibrary gles_library = LoadLibrary("libGLESv2.so.2");
  if (!gles_library)
    return false;
  base::NativeLibrary egl_library = LoadLibrary("libEGL.so.1");
  if (!egl_library) {
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

  SetGLGetProcAddressProc(get_proc_address);
  AddGLNativeLibrary(egl_library);
  AddGLNativeLibrary(gles_library);

  return true;
}

}  // namespace gfx

namespace ui {

void SurfaceFactoryWayland::InitializeWaylandEvent()
{
  e_factory = new EventFactoryWayland();
  EventFactoryWayland::SetInstance(e_factory);
}

SurfaceFactoryWayland::SurfaceFactoryWayland()
    : e_factory(NULL),
      root_window_(NULL),
      spec_(NULL)
{
  WaylandDisplay::Connect();
  LOG(INFO) << "Ozone: SurfaceFactoryWayland";
}

SurfaceFactoryWayland::~SurfaceFactoryWayland()
{
  if (spec_)
    delete[] spec_;

  WaylandDisplay::DestroyDisplay();
}

intptr_t SurfaceFactoryWayland::InitializeHardware()
{
  return (intptr_t) WaylandDisplay::GetDisplay()->display();
}

void SurfaceFactoryWayland::ShutdownHardware() {
  if (root_window_) {
    delete root_window_;
    root_window_ =NULL;
  }

  WaylandDisplay::DestroyDisplay();
}

gfx::AcceleratedWidget SurfaceFactoryWayland::GetAcceleratedWidget() {

  if (!e_factory)
    InitializeWaylandEvent();

  if (!root_window_)  {
    root_window_ = new WaylandWindow();
    root_window_->SetParentWindow(NULL);
  }

  return (gfx::AcceleratedWidget)root_window_;
}

gfx::AcceleratedWidget SurfaceFactoryWayland::RealizeAcceleratedWidget(
    gfx::AcceleratedWidget w) {
  ui::WaylandWindow* window = (ui::WaylandWindow*)w;

  return (gfx::AcceleratedWidget)window->egl_window();
}

const char* SurfaceFactoryWayland::DefaultDisplaySpec() {
  gfx::Rect scrn = WaylandDisplay::GetDisplay()->PrimaryScreen()->Geometry();
  int size = 2 * sizeof scrn.width();
  if (!spec_)
    spec_ = new char[size];

  base::snprintf(spec_, size, "%dx%d", scrn.width(), scrn.height());

  return spec_;
}

gfx::Screen* SurfaceFactoryWayland::CreateDesktopScreen() {
  return new DesktopScreenWayland;
}

bool SurfaceFactoryWayland::LoadEGLGLES2Bindings() {
  return gfx::InitializeGLBindings();
}

bool SurfaceFactoryWayland::AttemptToResizeAcceleratedWidget(
    gfx::AcceleratedWidget w,
    const gfx::Rect& bounds) {
  ui::WaylandWindow* window = (ui::WaylandWindow*)w;
  window->SetBounds(bounds);

  return true;
}

bool SurfaceFactoryWayland::AcceleratedWidgetCanBeResized(gfx::AcceleratedWidget w)
{
  return false;
}

gfx::VSyncProvider* SurfaceFactoryWayland::GetVSyncProvider(gfx::AcceleratedWidget w) {
  return 0;
}

}  // namespace ui
