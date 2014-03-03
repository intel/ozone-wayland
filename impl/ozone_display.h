// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_IMPL_OZONE_DISPLAY_H_
#define OZONE_IMPL_OZONE_DISPLAY_H_

#include "ozone/ui/events/output_change_observer.h"
#include "ui/gfx/native_widget_types.h"

namespace gfx {
class Screen;
}

namespace views {
class DesktopScreenWayland;
}

namespace ozonewayland {

class EventConverterOzoneWayland;
class OzoneDisplayChannel;
class OzoneDisplayChannelHost;
class WaylandDisplay;
class WaylandWindow;

class OzoneDisplay : public OutputChangeObserver {
 public:
  static OzoneDisplay* GetInstance();

  OzoneDisplay();
  virtual ~OzoneDisplay();

  const char* DefaultDisplaySpec();
  bool InitializeHardware();
  void ShutdownHardware();
  intptr_t GetNativeDisplay();

  gfx::Screen* CreateDesktopScreen();
  const views::DesktopScreenWayland* GetPrimaryScreen() const;

  gfx::AcceleratedWidget GetAcceleratedWidget();
  gfx::AcceleratedWidget RealizeAcceleratedWidget(gfx::AcceleratedWidget w);

  // OutputChangeObserver overrides.
  virtual void OnOutputSizeChanged(unsigned width, unsigned height) OVERRIDE;

 private:
  WaylandWindow* GetWidget(gfx::AcceleratedWidget w);
  void Terminate();
  void LookAheadOutputGeometry();

  static void DelayedInitialization(OzoneDisplay* display);

  views::DesktopScreenWayland* desktop_screen_;
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
