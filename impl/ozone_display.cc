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
#include "ozone/wayland/egl/loader.h"

#include "ozone/impl/ipc/child_process_observer.h"
#include "ozone/impl/ipc/display_channel.h"
#include "ozone/impl/ipc/display_channel_host.h"

#include "base/command_line.h"
#include "content/public/common/content_switches.h"

namespace ozonewayland {

OzoneDisplay* OzoneDisplay::instance_ = NULL;

OzoneDisplay* OzoneDisplay::GetInstance()
{
  return instance_;
}

OzoneDisplay::OzoneDisplay() : launch_type_(None),
    process_type_(PreLaunch),
    initialized_(false),
    state_(UnInitialized),
    dispatcher_(NULL),
    display_(NULL),
    root_window_(NULL),
    child_process_observer_(NULL),
    channel_(NULL),
    host_(NULL),
    e_factory_(NULL),
    spec_(NULL)
{
  instance_ = this;
}

ui::SurfaceFactoryOzone::HardwareState OzoneDisplay::InitializeHardware()
{
 if (state_ & Initialized)
   return initialized_state_;

 DCHECK(base::MessageLoop::current());
 base::MessageLoop::current()->AddDestructionObserver(this);

 state_ |= Initialized;
 ValidateLaunchType();
 bool gpuProcess = (launch_type_ & MultiProcess) && (process_type_ & Gpu);
 bool singleProcess = launch_type_ & SingleProcess;
 bool browserProcess = (launch_type_ & MultiProcess) && (process_type_ & Browser);

 if (singleProcess || gpuProcess) {
   display_ = new WaylandDisplay();
   initialized_state_ = display_->display() ? ui::SurfaceFactoryOzone::INITIALIZED
                                            : ui::SurfaceFactoryOzone::FAILED;
 }

 if (singleProcess || browserProcess)
   dispatcher_ = new WaylandDispatcher();

 if (singleProcess) {
   e_factory_ = new EventFactoryWayland();
   EventFactoryWayland::SetInstance(e_factory_);
 } else if (gpuProcess) {
   int fd = wl_display_get_fd(display_->display());
   dispatcher_ = new WaylandDispatcher(fd);
   channel_ = new OzoneDisplayChannel(fd);
   dispatcher_->PostTask(WaylandDispatcher::Poll);
 } else if (browserProcess) {
   child_process_observer_ = new OzoneProcessObserver(this);
   initialized_state_ = ui::SurfaceFactoryOzone::INITIALIZED;
 }

 // TODO(kalyan): Find a better way to set a default preferred size.
 gfx::Rect scrn = gfx::Rect(0, 0, 1,1);
 int size = 2 * sizeof scrn.width();
 spec_ = new char[size];
 base::snprintf(spec_, size, "%dx%d", scrn.width(), scrn.height());
 state_ |= PendingOutPut;

 if (initialized_state_ != ui::SurfaceFactoryOzone::INITIALIZED)
   LOG(ERROR) << "OzoneDisplay failed to initialize hardware";

 return initialized_state_;

}

void OzoneDisplay::ShutdownHardware()
{
  Terminate();
}

OzoneDisplay::~OzoneDisplay()
{
  Terminate();
  instance_ = NULL;
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

  if (root_window_) {
    delete root_window_;
    root_window_ =NULL;
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

  if (display_) {
    delete display_;
    display_ = NULL;
  }
}

intptr_t OzoneDisplay::GetNativeDisplay()
{
  return (intptr_t)display_->display();
}

gfx::AcceleratedWidget OzoneDisplay::GetAcceleratedWidget()
{
  if (!root_window_)
    root_window_ = new WaylandWindow(display_ ? WaylandWindow::TOPLEVEL
                                              : WaylandWindow::None);

  return (gfx::AcceleratedWidget)root_window_->Handle();
}

gfx::AcceleratedWidget OzoneDisplay::RealizeAcceleratedWidget(
    gfx::AcceleratedWidget w) {
  // TODO(kalyan): Map w to window.
  if (!root_window_)
    root_window_ = new WaylandWindow();

  root_window_->RealizeAcceleratedWidget();

  return (gfx::AcceleratedWidget)root_window_->egl_window();
}

const char* OzoneDisplay::DefaultDisplaySpec() {
  return spec_;
}

bool OzoneDisplay::AttemptToResizeAcceleratedWidget(
    gfx::AcceleratedWidget w,
    const gfx::Rect& bounds) {
  // TODO(kalyan): Map w to window.
  if (!(state_ &ChannelConnected) && channel_)
    channel_->Register();

  if (state_ & PendingOutPut) {
    // TODO(kalyan): AttemptToResizeAcceleratedWidget can be called during
    // pre-initialization phase and hence wayland events might not have been
    // handled. Fix this properly after understanding how defaultspec effects
    // layouting and if we can force OutputHandleMode to be handled immediately.

    // We are yet to get output geometry, instead of having some dummy value
    // assign size of root window to defaultspec.
    int size = 2 * sizeof bounds.width();
    base::snprintf(spec_, size, "%dx%d", bounds.width(), bounds.height());
  }

  return root_window_->SetBounds(bounds);
}

gfx::VSyncProvider* OzoneDisplay::GetVSyncProvider(
    gfx::AcceleratedWidget w) {
  return 0;
}

bool OzoneDisplay::SchedulePageFlip(gfx::AcceleratedWidget w) {
  // TODO(Kalyan): Map w to window.
  root_window_->HandleSwapBuffers();

  return true;
}

gfx::Screen* OzoneDisplay::CreateDesktopScreen() {
  return new DesktopScreenWayland;
}

bool OzoneDisplay::LoadEGLGLES2Bindings() {
  return InitializeGLBindings();
}

void OzoneDisplay::EstablishChannel(unsigned id)
{
  if (!host_)
    host_ = new OzoneDisplayChannelHost();

  host_->EstablishChannel(id);
}

void OzoneDisplay::OnChannelEstablished(unsigned id)
{
  state_ |= ChannelConnected;
  if (channel_) {
    gfx::Rect rect = display_->PrimaryScreen()->Geometry();
    dispatcher_->OutputSizeChanged(rect.width(), rect.height());
  }
}

void OzoneDisplay::OnChannelClosed(unsigned id)
{
  state_ &= ~ChannelConnected;
  if (!host_)
    return;

  host_->ChannelClosed(id);
}

void OzoneDisplay::OnChannelHostDestroyed()
{
  state_ &= ~ChannelConnected;
  host_ = NULL;
}

void OzoneDisplay::OnOutputSizeChanged(WaylandScreen* screen, int width,
                                       int height)
{
  if (screen == display_->PrimaryScreen()) {
    int size = 2 * sizeof width;
    base::snprintf(spec_, size, "%dx%d", width, height);
    state_ &= ~PendingOutPut;
    if (channel_ && (state_ & ChannelConnected))
      dispatcher_->OutputSizeChanged(width, height);
  }
}

void OzoneDisplay::OnOutputSizeChanged(unsigned width, unsigned height)
{
  if (host_) {
    int size = 2 * sizeof width;
    base::snprintf(spec_, size, "%dx%d", width, height);
    state_ &= ~PendingOutPut;
  }
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

void OzoneDisplay::ValidateLaunchType()
{
  const CommandLine& command_line = *CommandLine::ForCurrentProcess();
  bool singleProcess = command_line.HasSwitch(switches::kSingleProcess) ||
      command_line.HasSwitch(switches::kInProcessGPU) ||
      command_line.HasSwitch(switches::kTestCompositor);

  if (singleProcess) {
    launch_type_ = SingleProcess;
    process_type_ = Browser;
    return;
  }

  // We are not using single process mode, check if we are in browser process.
  if (!command_line.HasSwitch(switches::kProcessType)) {
    launch_type_ = MultiProcess;
    process_type_ = Browser;
    return;
  }

  bool gpuProcess = command_line.HasSwitch(switches::kProcessType) &&
      (command_line.HasSwitch(switches::kGpuDeviceID) ||
       command_line.HasSwitch(switches::kGpuDriverVendor) ||
       command_line.HasSwitch(switches::kGpuDriverVersion));

  if (gpuProcess) {
    launch_type_ = MultiProcess;
    process_type_ = Gpu;
  }
}

}  // namespace ozonewayland
