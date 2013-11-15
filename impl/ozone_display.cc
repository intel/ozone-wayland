// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/dispatcher.h"
#include "ozone/impl/ozone_display.h"

#include "ozone/impl/desktop_screen_wayland.h"
#include "ozone/impl/event_factory_wayland.h"
#include "ozone/wayland/display.h"
#include "ozone/wayland/dispatcher.h"
#include "ozone/wayland/screen.h"
#include "ozone/wayland/window.h"
#include "ozone/wayland/egl/egl_window.h"

#include "ozone/impl/ipc/child_process_observer.h"
#include "ozone/impl/ipc/display_channel.h"
#include "ozone/impl/ipc/display_channel_host.h"

#include "base/native_library.h"
#include "base/stl_util.h"

namespace ozonewayland {

OzoneDisplay* OzoneDisplay::instance_ = NULL;

OzoneDisplay* OzoneDisplay::GetInstance()
{
  return instance_;
}

OzoneDisplay::OzoneDisplay() : initialized_(false),
    state_(UnInitialized),
    desktop_screen_(NULL),
    dispatcher_(NULL),
    display_(NULL),
    child_process_observer_(NULL),
    channel_(NULL),
    host_(NULL),
    e_factory_(NULL),
    spec_(NULL),
    kMaxDisplaySize_(20)
{
  instance_ = this;
}

OzoneDisplay::~OzoneDisplay()
{
  Terminate();
  instance_ = NULL;
}

const char* OzoneDisplay::DefaultDisplaySpec() {
  if (spec_[0] == '\0')
    NOTREACHED() << "OutputHandleMode should come from Wayland compositor first";

  return spec_;
}

gfx::Screen* OzoneDisplay::CreateDesktopScreen() {
  if (!desktop_screen_) {
    desktop_screen_ = new DesktopScreenWayland;
    LookAheadOutputGeometry();
  }

  return desktop_screen_;
}

gfx::SurfaceFactoryOzone::HardwareState OzoneDisplay::InitializeHardware()
{
  if (state_ & Initialized)
    return initialized_state_;

  DCHECK(base::MessageLoop::current());
  base::MessageLoop::current()->AddDestructionObserver(this);

  state_ |= Initialized;
  display_ = new WaylandDisplay(WaylandDisplay::RegisterAsNeeded);
  initialized_state_ = display_->display() ? gfx::SurfaceFactoryOzone::INITIALIZED
                                           : gfx::SurfaceFactoryOzone::FAILED;

  if (initialized_state_ != gfx::SurfaceFactoryOzone::INITIALIZED)
    LOG(ERROR) << "OzoneDisplay failed to initialize hardware";

  return initialized_state_;
}

intptr_t OzoneDisplay::GetNativeDisplay()
{
  return (intptr_t)display_->display();
}

void OzoneDisplay::ShutdownHardware()
{
  Terminate();
}

gfx::AcceleratedWidget OzoneDisplay::GetAcceleratedWidget()
{
  static int opaque_handle = 0;
  // Ensure dispatcher is initialized.
  if (!dispatcher_)
    InitializeDispatcher();

  opaque_handle++;
  CreateWidget(opaque_handle);

  return (gfx::AcceleratedWidget)opaque_handle;
}

gfx::AcceleratedWidget OzoneDisplay::RealizeAcceleratedWidget(
    gfx::AcceleratedWidget w) {
  // Dispatcher should be already initialized unless we are in gpu process side.
  // Initialize dispatcher and start polling for wayland events.
  if (!dispatcher_)
    InitializeDispatcher(display_->GetDisplayFd());

  // TODO(kalyan) The channel connection should be established as soon as
  // GPU thread is initialized.
  if (!(state_ & ChannelConnected) && channel_) {
    channel_->Register();
    return gfx::kNullAcceleratedWidget;
  }

  WaylandWindow* widget = GetWidget(w);
  DCHECK(widget);
  widget->RealizeAcceleratedWidget();
  return (gfx::AcceleratedWidget)widget->egl_window();
}

bool OzoneDisplay::LoadEGLGLES2Bindings(
      gfx::SurfaceFactoryOzone::AddGLLibraryCallback add_gl_library,
      gfx::SurfaceFactoryOzone::SetGLGetProcAddressProcCallback set_proc_address) {
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

  set_proc_address.Run(get_proc_address);
  add_gl_library.Run(egl_library);
  add_gl_library.Run(gles_library);
  return true;
}

bool OzoneDisplay::AttemptToResizeAcceleratedWidget(gfx::AcceleratedWidget w,
                                                    const gfx::Rect& bounds) {
  if (host_) {
    host_->SendWidgetState(w, Resize, bounds.width(), bounds.height());
    return true;
  }

  WaylandWindow* window = GetWidget(w);
  // TODO(kalyan): Handle may be a opaque handle or a realized widget.
  // Fix this properly once resizing support is added.
  if (!window) {
    std::map<unsigned, WaylandWindow*>::const_iterator it;
    for (it = widget_map_.begin(); it != widget_map_.end(); ++it) {
      if (w == (gfx::AcceleratedWidget)it->second->egl_window()) {
        window = it->second;
        break;
      }
    }
  }

  DCHECK(window);

  return window->SetBounds(bounds);
}

gfx::VSyncProvider* OzoneDisplay::GetVSyncProvider(gfx::AcceleratedWidget w) {
  return 0;
}

bool OzoneDisplay::SchedulePageFlip(gfx::AcceleratedWidget w) {
  return true;
}

const int32* OzoneDisplay::GetEGLSurfaceProperties(const int32* desired_list)
{
  return EGLWindow::GetEGLConfigAttribs();
}

void OzoneDisplay::WillDestroyCurrentMessageLoop()
{
  DCHECK(base::MessageLoop::current());
  dispatcher_->MessageLoopDestroyed();

  if (child_process_observer_)
    child_process_observer_->WillDestroyCurrentMessageLoop();

  if (e_factory_)
    e_factory_->WillDestroyCurrentMessageLoop();

  base::MessageLoop::current()->RemoveDestructionObserver(this);
}

void OzoneDisplay::SetWidgetState(gfx::AcceleratedWidget w,
                                  WidgetState state,
                                  unsigned width,
                                  unsigned height)
{
  if (host_)
    host_->SendWidgetState(w, state, width, height);
  else
    OnWidgetStateChanged(w, state, width, height);
}

void OzoneDisplay::OnWidgetStateChanged(gfx::AcceleratedWidget w,
                                       WidgetState state,
                                       unsigned width,
                                       unsigned height)
{
  switch (state) {
    case Create:
      CreateWidget(w);
    case FullScreen:
      NOTIMPLEMENTED();
      break;
    case Maximized:
    {
      WaylandWindow* widget = GetWidget(w);
      widget->Maximize();
      break;
    }
    case Minimized:
    {
      WaylandWindow* widget = GetWidget(w);
      widget->Minimize();
      break;
    }
    case Restore:
    {
      WaylandWindow* widget = GetWidget(w);
      widget->Restore();
      break;
    }
    case Active:
      NOTIMPLEMENTED();
      break;
    case InActive:
      NOTIMPLEMENTED();
      break;
    case Show:
      NOTIMPLEMENTED();
      break;
    case Hide:
      NOTIMPLEMENTED();
      break;
    case Resize:
      AttemptToResizeAcceleratedWidget(w, gfx::Rect(0,0,width, height));
      break;
    default:
      break;
  }
}

void OzoneDisplay::SetWidgetTitle(gfx::AcceleratedWidget w,
                                  const string16& title) {
  if (host_)
    host_->SendWidgetTitle(w, title);
  else
    OnWidgetTitleChanged(w, title);
}

void OzoneDisplay::OnWidgetTitleChanged(gfx::AcceleratedWidget w,
                                  const string16& title) {
  WaylandWindow* widget = GetWidget(w);
  DCHECK(w);
  widget->SetWindowTitle(title);
}

void OzoneDisplay::EstablishChannel()
{
  if (state_ & ChannelConnected)
    return;

  if (!host_)
    host_ = new OzoneDisplayChannelHost();

  host_->EstablishChannel();
  state_ |= ChannelConnected;
}

void OzoneDisplay::OnChannelEstablished()
{
  state_ |= ChannelConnected;
}

void OzoneDisplay::OnChannelClosed()
{
  state_ &= ~ChannelConnected;
  if (!host_)
    return;

  host_->ChannelClosed();
}

void OzoneDisplay::OnChannelHostDestroyed()
{
  state_ &= ~ChannelConnected;
  host_ = NULL;
}

void OzoneDisplay::OnOutputSizeChanged(WaylandScreen* screen,
                                       int width,
                                       int height)
{
  if (!(state_ & Initialized))
    return;

  if (screen != display_->PrimaryScreen()) {
    NOTIMPLEMENTED () << "Multiple screens are not implemented";
    return;
  }

  if (channel_ && (state_ & ChannelConnected))
    dispatcher_->OutputSizeChanged(width, height);
  else
    OnOutputSizeChanged(width, height);
}

void OzoneDisplay::OnOutputSizeChanged(unsigned width, unsigned height)
{
  if (spec_)
    base::snprintf(spec_, kMaxDisplaySize_, "%dx%d*2", width, height);
  if (desktop_screen_)
    desktop_screen_->SetGeometry(gfx::Rect(0, 0, width, height));
}

WaylandWindow* OzoneDisplay::CreateWidget(unsigned w)
{
  WaylandWindow* window = NULL;
  if (!host_) {
    window = new WaylandWindow(display_ ? WaylandWindow::TOPLEVEL
                                        : WaylandWindow::None);
    widget_map_[w] = window;
  } else
    host_->SendWidgetState(w, Create, 0, 0);

  return window;
}

WaylandWindow* OzoneDisplay::GetWidget(gfx::AcceleratedWidget w)
{
  std::map<unsigned, WaylandWindow*>::const_iterator it = widget_map_.find(w);
  return it == widget_map_.end() ? NULL : it->second;
}

void OzoneDisplay::Terminate()
{
  if (!(state_ & Initialized))
    return;

  state_ &= ~Initialized;
  if (spec_) {
    delete[] spec_;
    spec_ = NULL;
  }

  if (widget_map_.size()) {
    STLDeleteValues(&widget_map_);
    widget_map_.clear();
  }

  if (channel_) {
    delete channel_;
    channel_ = NULL;
  }

  if (child_process_observer_) {
    delete child_process_observer_;
    child_process_observer_ = NULL;
  }

  if (dispatcher_) {
    delete dispatcher_;
    dispatcher_ = NULL;
  }

  if (desktop_screen_) {
    delete desktop_screen_;
    desktop_screen_ = NULL;
  }

  if (display_) {
    delete display_;
    display_ = NULL;
  }
}

void OzoneDisplay::InitializeDispatcher(int fd)
{
  dispatcher_ = new WaylandDispatcher(fd);

  if (fd) {
    channel_ = new OzoneDisplayChannel();
    dispatcher_->PostTask(WaylandDispatcher::Poll);
  } else {
    spec_ = new char[kMaxDisplaySize_];
    spec_[0] = '\0';

    if (display_)
      e_factory_ = new EventFactoryWayland();
    else {
      // Create OzoneDisplayChannelHost here to make sure we queue any requests
      // coming from DRWH before channel is established.
      host_ = new OzoneDisplayChannelHost();
      child_process_observer_ = new OzoneProcessObserver(this);
      EstablishChannel();
    }
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
