// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/impl/ozone_display.h"

#include <map>
#include "content/child/child_process.h"
#include "ozone/impl/desktop_screen_wayland.h"
#include "ozone/impl/ipc/display_channel.h"
#include "ozone/impl/ipc/display_channel_host.h"
#include "ozone/ui/events/event_converter_in_process.h"
#include "ozone/ui/events/remote_event_dispatcher.h"
#include "ozone/wayland/display.h"
#include "ozone/wayland/screen.h"
#include "ozone/wayland/window.h"

namespace ozonewayland {

OzoneDisplay* OzoneDisplay::instance_ = NULL;

OzoneDisplay* OzoneDisplay::GetInstance() {
  return instance_;
}

OzoneDisplay::OzoneDisplay() : desktop_screen_(NULL),
    display_(NULL),
    channel_(NULL),
    host_(NULL),
    event_converter_(NULL) {
  instance_ = this;
}

OzoneDisplay::~OzoneDisplay() {
  Terminate();
  instance_ = NULL;
}

bool OzoneDisplay::InitializeHardware() {
  display_ = new WaylandDisplay(WaylandDisplay::RegisterAsNeeded);
  bool initialized_hardware = display_->display() ? true : false;
  if (initialized_hardware && !content::ChildProcess::current()) {
    // In the multi-process mode, DisplayChannel (in GPU process side) is in
    // charge of establishing an IPC channel with DisplayChannelHost (in
    // Browser Process side). At this moment the GPU process is still
    // initializing though, so DisplayChannel cannot establish the connection
    // and need to delay this to later. Therefore post a task to GpuChildThread
    // and let DisplayChannel handle this right after the GPU process is
    // initialized.
    base::MessageLoop::current()->message_loop_proxy()->PostTask(
        FROM_HERE, base::Bind(&OzoneDisplay::DelayedInitialization, this));
  }

  return initialized_hardware;
}

void OzoneDisplay::ShutdownHardware() {
  Terminate();
}

intptr_t OzoneDisplay::GetNativeDisplay() {
  return (intptr_t)display_->display();
}

gfx::Screen* OzoneDisplay::CreateDesktopScreen() {
  if (!desktop_screen_) {
    desktop_screen_ = new views::DesktopScreenWayland;
    LookAheadOutputGeometry();
  }

  return desktop_screen_;
}

const views::DesktopScreenWayland* OzoneDisplay::GetPrimaryScreen() const {
  // TODO(kalyan): For now always return DesktopScreen. Needs proper fixing
  // after multi screen support is added.
  return desktop_screen_;
}

gfx::AcceleratedWidget OzoneDisplay::GetAcceleratedWidget() {
  static int opaque_handle = 0;
  // Ensure Event Converter is initialized.
  if (!event_converter_) {
    event_converter_ = new EventConverterInProcess();
    event_converter_->SetOutputChangeObserver(this);

    if (display_) {
      display_->StartProcessingEvents();
    } else {
      DCHECK(!host_);
      host_ = new OzoneDisplayChannelHost();
    }
  }

  opaque_handle++;
  WindowStateChangeHandler::GetInstance()->SetWidgetState(opaque_handle,
                                                          CREATE,
                                                          0,
                                                          0);

  return (gfx::AcceleratedWidget)opaque_handle;
}

gfx::AcceleratedWidget OzoneDisplay::RealizeAcceleratedWidget(
    gfx::AcceleratedWidget w) {
  DCHECK(display_);
  // Event Converter should be already initialized unless we are in gpu process
  // side.
  if (!event_converter_) {
    event_converter_ = new RemoteEventDispatcher();
    display_->StartProcessingEvents();
  }

  WaylandWindow* widget = GetWidget(w);
  DCHECK(widget);
  widget->RealizeAcceleratedWidget();
  return (gfx::AcceleratedWidget)widget->egl_window();
}

void OzoneDisplay::OnOutputSizeChanged(unsigned width, unsigned height) {
  if (desktop_screen_)
    desktop_screen_->SetGeometry(gfx::Rect(0, 0, width, height));
}

void OzoneDisplay::DelayedInitialization(OzoneDisplay* display) {
  display->channel_ = new OzoneDisplayChannel();
  display->channel_->Register();
}

WaylandWindow* OzoneDisplay::GetWidget(gfx::AcceleratedWidget w) {
  const std::map<unsigned, WaylandWindow*> widget_map =
      display_->GetWindowList();

  std::map<unsigned, WaylandWindow*>::const_iterator it = widget_map.find(w);
    return it == widget_map.end() ? NULL : it->second;
}

void OzoneDisplay::Terminate() {
  if (!event_converter_ && !desktop_screen_)
    return;

  delete channel_;
  if (desktop_screen_) {
    delete desktop_screen_;
    desktop_screen_ = NULL;
  }

  delete display_;
  if (event_converter_) {
    delete event_converter_;
    event_converter_ = NULL;
  }
}

// TODO(vignatti): GPU process conceptually is the one that deals with hardware
// details and therefore we assume that the window system connection should
// happen in there only. There's a glitch with Chrome though, that creates its
// frame contents requiring access to the window system, before the GPU process
// even exists. In other words, Chrome runs
// BrowserMainLoop::PreMainMessageLoopRun before GpuProcessHost::Get. If the
// assumption of window system connection belongs to the GPU process is valid,
// then I believe this Chrome behavior needs to be addressed upstream.
//
// For now, we create another window system connection to look ahead the needed
// output properties that Chrome (among others) need and then close right after
// that. I haven't measured how long it takes to open a Wayland connection,
// listen all the interface the compositor sends and close it, but _for_ _sure_
// it slows down the overall initialization time of Chromium targets.
// Therefore, this is something that has to be solved in the future, moving all
// Chrome tasks after GPU process is created.
//
void OzoneDisplay::LookAheadOutputGeometry() {
  DCHECK(desktop_screen_);
  WaylandDisplay disp_(WaylandDisplay::RegisterOutputOnly);
  CHECK(disp_.display()) << "Ozone: Wayland server connection not found.";

  while (disp_.PrimaryScreen()->Geometry().IsEmpty())
    disp_.SyncDisplay();

  desktop_screen_->SetGeometry(disp_.PrimaryScreen()->Geometry());
}

}  // namespace ozonewayland
