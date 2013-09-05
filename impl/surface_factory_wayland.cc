// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/impl/surface_factory_wayland.h"

#include "base/base_paths.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/native_library.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/string_util.h"
#include "ozone/impl/desktop_screen_wayland.h"
#include "ozone/impl/event_factory_wayland.h"
#include "ozone/wayland/display.h"
#include "ozone/wayland/window.h"
#include "ozone/wayland/screen.h"
#include "ozone/wayland/egl/loader.h"

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
  LOG(INFO) << "Ozone: SurfaceFactoryWayland";
}

SurfaceFactoryWayland::~SurfaceFactoryWayland()
{
  if (spec_)
    delete[] spec_;

  WaylandDisplay::DestroyDisplay();
}

SurfaceFactoryOzone::HardwareState SurfaceFactoryWayland::InitializeHardware()
{
  if (!WaylandDisplay::GetDisplay())
    WaylandDisplay::Connect();

  return WaylandDisplay::GetDisplay()->display() ? SurfaceFactoryOzone::INITIALIZED : SurfaceFactoryOzone::FAILED;
}

intptr_t SurfaceFactoryWayland::GetNativeDisplay()
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

  if (!root_window_)
    root_window_ = new WaylandWindow();

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
