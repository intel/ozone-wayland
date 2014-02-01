// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/impl/ozone_display.h"

#include <map>
#include <string>

#include "base/native_library.h"
#include "content/child/child_process.h"
#include "ozone/impl/desktop_screen_wayland.h"
#include "ozone/impl/vsync_provider_wayland.h"
#include "ozone/impl/ipc/display_channel.h"
#include "ozone/impl/ipc/display_channel_host.h"
#include "ozone/ui/events/event_converter_in_process.h"
#include "ozone/ui/events/remote_event_dispatcher.h"
#include "ozone/wayland/display.h"
#include "ozone/wayland/egl/egl_window.h"
#include "ozone/wayland/screen.h"
#include "ozone/wayland/window.h"
#include "ui/gl/sync_control_vsync_provider.h"

namespace ozonewayland {

OzoneDisplay* OzoneDisplay::instance_ = NULL;

OzoneDisplay* OzoneDisplay::GetInstance() {
  return instance_;
}

OzoneDisplay::OzoneDisplay() : state_(UnInitialized),
    initialized_state_(gfx::SurfaceFactoryOzone::INITIALIZED),
    last_realized_widget_handle_(0),
    kMaxDisplaySize_(20),
    desktop_screen_(NULL),
    display_(NULL),
    channel_(NULL),
    host_(NULL),
    event_converter_(NULL),
    spec_(NULL) {
  instance_ = this;
}

OzoneDisplay::~OzoneDisplay() {
  Terminate();
  instance_ = NULL;
}

const char* OzoneDisplay::DefaultDisplaySpec() {
  if (!spec_) {
    spec_ = new char[kMaxDisplaySize_];
    if (desktop_screen_ && !desktop_screen_->geometry().size().IsEmpty())  {
      gfx::Rect rect(desktop_screen_->geometry());
      base::snprintf(spec_,
                     kMaxDisplaySize_,
                     "%dx%d*2",
                     rect.width(),
                     rect.height());
    } else {
      spec_[0] = '\0';
    }
  }

  if (spec_[0] == '\0')
    NOTREACHED() <<
        "OutputHandleMode should come from Wayland compositor first";

  return spec_;
}

gfx::Screen* OzoneDisplay::CreateDesktopScreen() {
  if (!desktop_screen_) {
    desktop_screen_ = new DesktopScreenWayland;
    LookAheadOutputGeometry();
  }

  return desktop_screen_;
}

gfx::SurfaceFactoryOzone::HardwareState OzoneDisplay::InitializeHardware() {
  if (state_ & Initialized)
    return initialized_state_;

  state_ |= Initialized;
  display_ = new WaylandDisplay(WaylandDisplay::RegisterAsNeeded);
  initialized_state_ =
      display_->display() ? gfx::SurfaceFactoryOzone::INITIALIZED
                          : gfx::SurfaceFactoryOzone::FAILED;

  if (initialized_state_ != gfx::SurfaceFactoryOzone::INITIALIZED)
    LOG(ERROR) << "OzoneDisplay failed to initialize hardware";
  else if (!content::ChildProcess::current()) {
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

  return initialized_state_;
}

intptr_t OzoneDisplay::GetNativeDisplay() {
  return (intptr_t)display_->display();
}

void OzoneDisplay::ShutdownHardware() {
  Terminate();
}

gfx::AcceleratedWidget OzoneDisplay::GetAcceleratedWidget() {
  static int opaque_handle = 0;
  // Ensure Event Converter is initialized.
  if (!event_converter_)
    InitializeDispatcher();

  opaque_handle++;
  WindowStateChangeHandler::GetInstance()->SetWidgetState(opaque_handle,
                                                          CREATE,
                                                          0,
                                                          0);

  return (gfx::AcceleratedWidget)opaque_handle;
}

gfx::AcceleratedWidget OzoneDisplay::RealizeAcceleratedWidget(
    gfx::AcceleratedWidget w) {
  // Event Converter should be already initialized unless we are in gpu process
  // side.
  if (!event_converter_)
    InitializeDispatcher(display_->GetDisplayFd());

  WaylandWindow* widget = GetWidget(w);
  DCHECK(widget);
  last_realized_widget_handle_ = w;
  widget->RealizeAcceleratedWidget();
  return (gfx::AcceleratedWidget)widget->egl_window();
}

bool OzoneDisplay::LoadEGLGLES2Bindings(
    gfx::SurfaceFactoryOzone::AddGLLibraryCallback add_gl_library,
    gfx::SurfaceFactoryOzone::SetGLGetProcAddressProcCallback setprocaddress) {
  // The variable EGL_PLATFORM specifies native platform to be used by the
  // drivers (atleast on Mesa). When the variable is not set, Mesa uses the
  // first platform listed in --with-egl-platforms during compilation. Thus, we
  // ensure here that wayland is set as the native platform. However, we don't
  // override the EGL_PLATFORM value in case it has already been set.
  setenv("EGL_PLATFORM", "wayland", 0);
  std::string error;
  base::NativeLibrary gles_library = base::LoadNativeLibrary(
    base::FilePath("libGLESv2.so.2"), &error);

  if (!gles_library) {
    LOG(WARNING) << "Failed to load GLES library: " << error;
    return false;
  }

  base::NativeLibrary egl_library = base::LoadNativeLibrary(
    base::FilePath("libEGL.so.1"), &error);

  if (!egl_library) {
    LOG(WARNING) << "Failed to load EGL library: " << error;
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

  setprocaddress.Run(get_proc_address);
  add_gl_library.Run(egl_library);
  add_gl_library.Run(gles_library);
  return true;
}

bool OzoneDisplay::AttemptToResizeAcceleratedWidget(gfx::AcceleratedWidget w,
                                                    const gfx::Rect& bounds) {
  WindowStateChangeHandler::GetInstance()->SetWidgetState(w,
                                                          RESIZE,
                                                          bounds.width(),
                                                          bounds.height());

  return true;
}


scoped_ptr<gfx::VSyncProvider>
OzoneDisplay::CreateVSyncProvider(gfx::AcceleratedWidget w) {
  DCHECK(last_realized_widget_handle_);
  // This is based on the fact that we realize accelerated widget and create
  // its vsync provider immediately (right after widget is realized). This
  // saves us going through list of realized widgets and finding the right one.
  unsigned handle = last_realized_widget_handle_;
  last_realized_widget_handle_ = 0;
  return scoped_ptr<gfx::VSyncProvider>(new WaylandSyncProvider(handle));
}

bool OzoneDisplay::SchedulePageFlip(gfx::AcceleratedWidget w) {
  return true;
}

const int32* OzoneDisplay::GetEGLSurfaceProperties(const int32* desired_list) {
  return EGLWindow::GetEGLConfigAttribs();
}

void OzoneDisplay::OnOutputSizeChanged(unsigned width, unsigned height) {
  if (spec_)
    base::snprintf(spec_, kMaxDisplaySize_, "%dx%d*2", width, height);
  if (desktop_screen_)
    desktop_screen_->SetGeometry(gfx::Rect(0, 0, width, height));
}

const DesktopScreenWayland* OzoneDisplay::GetPrimaryScreen() const {
  // TODO(kalyan): For now always return DesktopScreen. Needs proper fixing
  // after multi screen support is added.
  return desktop_screen_;
}

void OzoneDisplay::EstablishChannel() {
  if (state_ & ChannelConnected)
    return;

  if (!host_)
    host_ = new OzoneDisplayChannelHost();

  host_->EstablishChannel();
  state_ |= ChannelConnected;
}

void OzoneDisplay::OnChannelEstablished() {
  state_ |= ChannelConnected;
}

void OzoneDisplay::OnChannelHostDestroyed() {
  state_ &= ~ChannelConnected;
  host_ = NULL;
}

void OzoneDisplay::DelayedInitialization(OzoneDisplay* display) {
  display->channel_ = new OzoneDisplayChannel();
  display->channel_->Register();
}

WaylandWindow* OzoneDisplay::GetWidget(gfx::AcceleratedWidget w) {
  DCHECK(display_);
  const std::map<unsigned, WaylandWindow*> widget_map =
      display_->GetWindowList();

  std::map<unsigned, WaylandWindow*>::const_iterator it = widget_map.find(w);
    return it == widget_map.end() ? NULL : it->second;
}

void OzoneDisplay::Terminate() {
  if (!(state_ & Initialized))
    return;

  state_ &= ~Initialized;
  if (spec_)
    delete[] spec_;

  delete channel_;
  delete desktop_screen_;
  delete display_;
  delete event_converter_;
}

void OzoneDisplay::InitializeDispatcher(int fd) {
  DCHECK(base::MessageLoop::current() && !event_converter_);
  if (fd) {
    event_converter_ = new RemoteEventDispatcher();
  } else {
    event_converter_ = new EventConverterInProcess();
    event_converter_->SetOutputChangeObserver(this);
  }

  if (display_) {
    display_->StartProcessingEvents();
  } else {
    EstablishChannel();
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
