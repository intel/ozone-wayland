// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/platform/ozone_platform_wayland.h"

#include "base/at_exit.h"
#include "base/bind.h"
#include "ozone/platform/ozone_wayland_window.h"
#include "ozone/ui/cursor/cursor_factory_ozone_wayland.h"
#include "ozone/ui/events/event_factory_ozone_wayland.h"
#include "ozone/ui/ime/input_method_context_factory_wayland.h"
#include "ozone/ui/public/ozone_channel.h"
#include "ozone/ui/public/ozone_channel_host.h"
#include "ozone/wayland/display.h"
#include "ui/events/ozone/layout/keyboard_layout_engine_manager.h"
#include "ui/events/ozone/layout/xkb/xkb_evdev_codes.h"
#include "ui/events/ozone/layout/xkb/xkb_keyboard_layout_engine.h"
#include "ui/ozone/common/native_display_delegate_ozone.h"
#include "ui/ozone/public/ozone_platform.h"
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

  virtual ~OzonePlatformWayland() {
  }

  // OzonePlatform:
  ui::SurfaceFactoryOzone* GetSurfaceFactoryOzone() override {
    return wayland_display_.get();
  }

  CursorFactoryOzone* GetCursorFactoryOzone() override {
    return cursor_factory_ozone_.get();
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
    return gpu_platform_.get();
  }

  scoped_ptr<PlatformWindow> CreatePlatformWindow(
      PlatformWindowDelegate* delegate,
      const gfx::Rect& bounds) override {
    return scoped_ptr<PlatformWindow>(new OzoneWaylandWindow(delegate,
                                                             bounds));
  }

  scoped_ptr<NativeDisplayDelegate> CreateNativeDisplayDelegate() override {
    return scoped_ptr<NativeDisplayDelegate>(new NativeDisplayDelegateOzone());
  }

  void InitializeUI() override {
    // For tests.
    if (wayland_display_.get())
      return;

    gpu_platform_host_.reset(new ui::OzoneChannelHost());
    // Needed as Browser creates accelerated widgets through SFO.
    wayland_display_.reset(new ozonewayland::WaylandDisplay());
    input_method_factory_.reset(
        new ui::InputMethodContextFactoryWayland());
    cursor_factory_ozone_.reset(new ui::CursorFactoryOzoneWayland());
    KeyboardLayoutEngineManager::SetKeyboardLayoutEngine(make_scoped_ptr(
        new XkbKeyboardLayoutEngine(xkb_evdev_code_converter_)));
    event_factory_ozone_.reset(
        new ui::EventFactoryOzoneWayland(
            gpu_platform_host_.get()));
  }

  void InitializeGPU() override {
    gpu_platform_.reset(new ui::OzoneChannel());
    if (!event_factory_ozone_)
      event_factory_ozone_.reset(new ui::EventFactoryOzoneWayland());

    if (!wayland_display_)
      wayland_display_.reset(new ozonewayland::WaylandDisplay());

    if (!wayland_display_->InitializeHardware())
      LOG(FATAL) << "failed to initialize display hardware";
  }

 private:
  scoped_ptr<ui::InputMethodContextFactoryWayland> input_method_factory_;
  scoped_ptr<ui::EventFactoryOzoneWayland> event_factory_ozone_;
  scoped_ptr<ui::CursorFactoryOzoneWayland> cursor_factory_ozone_;
  scoped_ptr<ui::OzoneChannelHost> gpu_platform_host_;
  scoped_ptr<ui::OzoneChannel> gpu_platform_;
  scoped_ptr<ozonewayland::WaylandDisplay> wayland_display_;
  XkbEvdevCodes xkb_evdev_code_converter_;
  DISALLOW_COPY_AND_ASSIGN(OzonePlatformWayland);
};

}  // namespace

OzonePlatform* CreateOzonePlatformWayland() { return new OzonePlatformWayland; }

}  // namespace ui
