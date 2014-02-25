// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_IMPL_OZONE_DISPLAY_H_
#define OZONE_IMPL_OZONE_DISPLAY_H_

#include "ui/gfx/native_widget_types.h"

namespace gfx {
class Screen;
}

namespace ozonewayland {

class DesktopScreenWayland;
class EventConverterOzoneWayland;
class OzoneDisplayChannel;
class OzoneDisplayChannelHost;
class WaylandDisplay;
class WaylandWindow;

class OzoneDisplay {
 public:
  static OzoneDisplay* GetInstance();

  OzoneDisplay();
  virtual ~OzoneDisplay();

  const char* DefaultDisplaySpec();
  bool InitializeHardware();
  void ShutdownHardware();
  intptr_t GetNativeDisplay();

  gfx::Screen* CreateDesktopScreen();

  gfx::AcceleratedWidget GetAcceleratedWidget();
  gfx::AcceleratedWidget RealizeAcceleratedWidget(gfx::AcceleratedWidget w);

 private:
  WaylandWindow* GetWidget(gfx::AcceleratedWidget w);
  void Terminate();
  void LookAheadOutputGeometry();

  static void DelayedInitialization(OzoneDisplay* display);

  DesktopScreenWayland* desktop_screen_;
  WaylandDisplay* display_;
  OzoneDisplayChannel* channel_;
  OzoneDisplayChannelHost* host_;
  EventConverterOzoneWayland* event_converter_;
  char* spec_;
  static OzoneDisplay* instance_;
  DISALLOW_COPY_AND_ASSIGN(OzoneDisplay);
};

}  // namespace ozonewayland

#endif  // OZONE_IMPL_OZONE_DISPLAY_H_
