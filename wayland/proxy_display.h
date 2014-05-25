// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_PROXY_DISPLAY_H_
#define OZONE_WAYLAND_PROXY_DISPLAY_H_

#if !defined(__STDC_FORMAT_MACROS)
#define __STDC_FORMAT_MACROS
#endif

#include <wayland-client.h>
#include "ozone/ui/gfx/ozone_display.h"

namespace ozonewayland {

class WaylandScreen;
// WaylandProxyDisplay is a wrapper around wl_display. Once we get a valid
// wl_display, the Wayland server will send different events to register
// the Wayland compositor, shell, screens, input devices, ...
class WaylandProxyDisplay : public gfx::OzoneDisplay {
 public:
  explicit WaylandProxyDisplay();
  virtual ~WaylandProxyDisplay();

  virtual void LookAheadOutputGeometry() OVERRIDE;

 private:
  // This handler resolves only screen registration. In general you don't want
  // to use this but the one below.
  static void DisplayHandleOutputOnly(
      void *data,
      struct wl_registry *registry,
      uint32_t name,
      const char *interface,
      uint32_t version);

  // WaylandProxyDisplay manages the memory of all these pointers.
  WaylandScreen* screen_;
  DISALLOW_COPY_AND_ASSIGN(WaylandProxyDisplay);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_PROXY_DISPLAY_H_
