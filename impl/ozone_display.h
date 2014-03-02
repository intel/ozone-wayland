// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_IMPL_OZONE_DISPLAY_H_
#define OZONE_IMPL_OZONE_DISPLAY_H_

#include "ui/gfx/native_widget_types.h"

namespace gfx {
class Screen;
}

namespace content {
class OzoneDisplayChannel;
class OzoneDisplayChannelHost;
}

namespace ozonewayland {

class WaylandDisplay;

class OzoneDisplay {
 public:
  static OzoneDisplay* GetInstance();

  OzoneDisplay();
  virtual ~OzoneDisplay();

  bool InitializeHardware();
  void ShutdownHardware();
  intptr_t GetNativeDisplay();

  gfx::AcceleratedWidget GetAcceleratedWidget();
  gfx::AcceleratedWidget RealizeAcceleratedWidget(gfx::AcceleratedWidget w);

 private:
  void Terminate();

  static void DelayedInitialization(OzoneDisplay* display);

  WaylandDisplay* display_;
  content::OzoneDisplayChannel* channel_;
  content::OzoneDisplayChannelHost* host_;
  static OzoneDisplay* instance_;
  DISALLOW_COPY_AND_ASSIGN(OzoneDisplay);
};

}  // namespace ozonewayland

#endif  // OZONE_IMPL_OZONE_DISPLAY_H_
