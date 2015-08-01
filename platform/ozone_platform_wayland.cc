// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/platform/ozone_platform_wayland.h"

#include "base/at_exit.h"
#include "base/bind.h"
#include "ozone/platform/ozone_gpu_platform_support_host.h"
#include "ozone/platform/ozone_wayland_window.h"
#include "ozone/platform/window_manager_wayland.h"
#include "ozone/wayland/display.h"
#include "ozone/wayland/ozone_wayland_screen.h"
#include "ui/base/cursor/ozone/bitmap_cursor_factory_ozone.h"
#include "ui/events/ozone/layout/keyboard_layout_engine_manager.h"
#include "ui/events/ozone/layout/xkb/xkb_evdev_codes.h"
#include "ui/events/ozone/layout/xkb/xkb_keyboard_layout_engine.h"
#include "ui/ozone/common/native_display_delegate_ozone.h"
#include "ui/ozone/common/stub_overlay_manager.h"
#include "ui/ozone/public/system_input_injector.h"
#include "ui/platform_window/platform_window_delegate.h"

namespace ui {

namespace {

// OzonePlatform for Wayland
//
// This platform is Linux with the Wayland display server.
class OzonePlatformWayland : public OzonePlatform {
 public:
  OzonePlatformWayland() {
    base::AtExitManager::RegisterTask(
        base::Bind(&base::DeletePointer<OzonePlatformWayland>, this));
  }

  ~OzonePlatformWayland() override {
  }

  // OzonePlatform:
  ui::SurfaceFactoryOzone* GetSurfaceFactoryOzone() override {
    return wayland_display_.get();
  }

  CursorFactoryOzone* GetCursorFactoryOzone() override {
    return cursor_factory_ozone_.get();
  }

  ui::OverlayManagerOzone* GetOverlayManager() override {
    return overlay_manager_.get();
  }

  InputController* GetInputController() override {
    return NULL;
  }

  scoped_ptr<SystemInputInjector> CreateSystemInputInjector() override {
    return scoped_ptr<SystemInputInjector>();
  }

  GpuPlatformSupportHost* GetGpuPlatformSupportHost() override {
    return gpu_platform_host_.get();
  }

  GpuPlatformSupport* GetGpuPlatformSupport() override {
    return wayland_display_.get();
  }

  scoped_ptr<PlatformWindow> CreatePlatformWindow(
      PlatformWindowDelegate* delegate,
      const gfx::Rect& bounds) override {
    return scoped_ptr<PlatformWindow>(
        new OzoneWaylandWindow(delegate,
                               gpu_platform_host_.get(),
                               window_manager_.get(),
                               bounds));
  }

  scoped_ptr<DesktopPlatformScreen> CreatePlatformScreen(
      DesktopPlatformScreenDelegate* delegate) {
    return scoped_ptr<DesktopPlatformScreen>(
         new ozonewayland::OzoneWaylandScreen(delegate,
                                              window_manager_.get()));
  }

  scoped_ptr<NativeDisplayDelegate> CreateNativeDisplayDelegate() override {
    return nullptr;
  }

  void InitializeUI() override {
    // For tests.
    if (wayland_display_.get())
      return;

    gpu_platform_host_.reset(new ui::OzoneGpuPlatformSupportHost());
    // Needed as Browser creates accelerated widgets through SFO.
    wayland_display_.reset(new ozonewayland::WaylandDisplay());
    cursor_factory_ozone_.reset(new ui::BitmapCursorFactoryOzone());
    overlay_manager_.reset(new StubOverlayManager());
    KeyboardLayoutEngineManager::SetKeyboardLayoutEngine(make_scoped_ptr(
        new XkbKeyboardLayoutEngine(xkb_evdev_code_converter_)));
    window_manager_.reset(
        new ui::WindowManagerWayland(gpu_platform_host_.get()));
  }

  void InitializeGPU() override {
    if (!wayland_display_)
      wayland_display_.reset(new ozonewayland::WaylandDisplay());

    if (!wayland_display_->InitializeHardware())
      LOG(FATAL) << "failed to initialize display hardware";
  }

 private:
  scoped_ptr<ui::BitmapCursorFactoryOzone> cursor_factory_ozone_;
  scoped_ptr<ozonewayland::WaylandDisplay> wayland_display_;
  scoped_ptr<StubOverlayManager> overlay_manager_;
  scoped_ptr<ui::WindowManagerWayland> window_manager_;
  XkbEvdevCodes xkb_evdev_code_converter_;
  scoped_ptr<ui::OzoneGpuPlatformSupportHost> gpu_platform_host_;
  DISALLOW_COPY_AND_ASSIGN(OzonePlatformWayland);
};

}  // namespace

OzonePlatform* CreateOzonePlatformWayland() { return new OzonePlatformWayland; }

scoped_ptr<DesktopPlatformScreen> CreatePlatformScreen(
    DesktopPlatformScreenDelegate* delegate) {
  OzonePlatformWayland* platform =
      static_cast<OzonePlatformWayland*>(ui::OzonePlatform::GetInstance());
  return platform->CreatePlatformScreen(delegate);
}

}  // namespace ui
