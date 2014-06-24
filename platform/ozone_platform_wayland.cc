// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/platform/ozone_platform_wayland.h"

#include "ozone/ui/cursor/cursor_factory_ozone_wayland.h"
#if defined(TOOLKIT_VIEWS) && !defined(OS_CHROMEOS)
#include "ozone/ui/desktop_aura/desktop_factory_wayland.h"
#endif
#include "ozone/ui/events/event_factory_ozone_wayland.h"
#include "ozone/ui/ime/input_method_context_factory_wayland.h"
#include "ozone/ui/public/ozone_channel.h"
#include "ozone/ui/public/ozone_channel_host.h"
#include "ozone/wayland/display.h"
#include "ozone/wayland/proxy_display.h"
#include "ui/ozone/ozone_platform.h"

namespace ui {

namespace {

// OzonePlatform for Wayland
//
// This platform is Linux with the Wayland display server.
class OzonePlatformWayland : public OzonePlatform {
 public:
  OzonePlatformWayland() : wayland_proxy_display_(NULL) {}

  virtual ~OzonePlatformWayland() {
    delete wayland_proxy_display_;
  }

  // OzonePlatform:
  virtual ui::SurfaceFactoryOzone* GetSurfaceFactoryOzone() OVERRIDE {
    return wayland_display_.get();
  }
  virtual EventFactoryOzone* GetEventFactoryOzone() OVERRIDE {
    return event_factory_ozone_.get();
  }
  virtual CursorFactoryOzone* GetCursorFactoryOzone() OVERRIDE {
    return cursor_factory_ozone_.get();
  }

  virtual GpuPlatformSupportHost* GetGpuPlatformSupportHost() OVERRIDE {
    return gpu_platform_host_.get();
  }

  virtual GpuPlatformSupport* GetGpuPlatformSupport() OVERRIDE {
    return gpu_platform_.get();
  }

  virtual void InitializeUI() OVERRIDE {
    event_factory_ozone_.reset(
        new ui::EventFactoryOzoneWayland());

    gpu_platform_host_.reset(new ui::OzoneChannelHost());
    input_method_factory_.reset(
        new ui::InputMethodContextFactoryWayland());
    cursor_factory_ozone_.reset(new ui::CursorFactoryOzoneWayland());
    wayland_proxy_display_ = new ozonewayland::WaylandProxyDisplay();
  }

  virtual void InitializeGPU() OVERRIDE {
    gpu_platform_.reset(new ui::OzoneChannel());
    if (!event_factory_ozone_) {
      event_factory_ozone_.reset(new ui::EventFactoryOzoneWayland());
      gpu_platform_.get()->InitializeRemoteDispatcher();
    } else {
    // We don't need proxy display in case of Single process.
    // TODO(kalyan): Find a better way to handle this.
      gpu_platform_host_.get()->DeleteRemoteStateChangeHandler();
      delete wayland_proxy_display_;
      wayland_proxy_display_ = NULL;
    }

    wayland_display_.reset(new ozonewayland::WaylandDisplay());
  }

 private:
#if defined(TOOLKIT_VIEWS) && !defined(OS_CHROMEOS)
  views::DesktopFactoryWayland desktop_factory_ozone_;
#endif
  ozonewayland::WaylandProxyDisplay* wayland_proxy_display_;
  scoped_ptr<ui::InputMethodContextFactoryWayland> input_method_factory_;
  scoped_ptr<ui::EventFactoryOzoneWayland> event_factory_ozone_;
  scoped_ptr<ui::CursorFactoryOzoneWayland> cursor_factory_ozone_;
  scoped_ptr<ui::OzoneChannelHost> gpu_platform_host_;
  scoped_ptr<ui::OzoneChannel> gpu_platform_;
  scoped_ptr<ozonewayland::WaylandDisplay> wayland_display_;
  DISALLOW_COPY_AND_ASSIGN(OzonePlatformWayland);
};

}  // namespace

OzonePlatform* CreateOzonePlatformWayland() { return new OzonePlatformWayland; }

}  // namespace ui
