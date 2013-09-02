// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/display.h"

#include "ozone/wayland/dispatcher.h"
#include "ozone/wayland/input_device.h"
#include "ozone/wayland/screen.h"
#include "ozone/wayland/window.h"
#include "ozone/wayland/cursor.h"

namespace ui {

WaylandDisplay* WaylandDisplay::m_instance = NULL;

WaylandDisplay* WaylandDisplay::GetDisplay() {
  return m_instance;
}

void WaylandDisplay::DestroyDisplay() {
  if (m_instance)
    m_instance->Terminate();
}

// static
WaylandDisplay* WaylandDisplay::ConnectDisplay() {

  if (!m_instance)
    m_instance = OpenStaticConnection();

  if (!m_instance->display())
    m_instance->Connect();

  return m_instance;
}

WaylandDisplay* WaylandDisplay::OpenStaticConnection()
{
  static WaylandDisplay display;
  return &display;
}

WaylandDisplay::WaylandDisplay()
    : display_(NULL),
    compositor_(NULL),
    shell_(NULL),
    shm_(NULL),
    primary_screen_(NULL),
    primary_input_device_(NULL),
    dispatcher_(NULL)
{
}

WaylandDisplay::~WaylandDisplay()
{
  Terminate();
}

void WaylandDisplay::Connect() {
  display_ = wl_display_connect(NULL);
  if (!display_)
      return;

  static const struct wl_registry_listener registry_listener = {
    WaylandDisplay::DisplayHandleGlobal
  };

  registry_ = wl_display_get_registry(display_);
  wl_registry_add_listener(registry_, &registry_listener, this);

  if (wl_display_roundtrip(display_) < 0) {
      Terminate();
      return;
  }

  wl_display_set_user_data(display_, this);
  dispatcher_ = new WaylandDispatcher(this);
}

void WaylandDisplay::Terminate()
{
  if (!display_)
    return;

  for (std::list<WaylandInputDevice*>::iterator i = input_list_.begin();
      i != input_list_.end(); ++i) {
      delete *i;
  }

  for (std::list<WaylandScreen*>::iterator i = screen_list_.begin();
      i != screen_list_.end(); ++i) {
      delete *i;
  }

  screen_list_.clear();
  input_list_.clear();

  WaylandCursor::Clear();

  if (compositor_)
    wl_compositor_destroy(compositor_);

  if (shell_)
    wl_shell_destroy(shell_);

  if (shm_)
    wl_shm_destroy(shm_);

  if (registry_)
      wl_registry_destroy(registry_);

  if (dispatcher_) {
    delete dispatcher_;
    dispatcher_ = NULL;
  }

  if (display_) {
    wl_display_flush(display_);
    wl_display_disconnect(display_);
    display_ = NULL;
  }
}

// static
void WaylandDisplay::DisplayHandleGlobal(void *data,
    struct wl_registry *registry,
    uint32_t name,
    const char *interface,
    uint32_t version)
{

  WaylandDisplay* disp = static_cast<WaylandDisplay*>(data);

  if (strcmp(interface, "wl_compositor") == 0) {
    disp->compositor_ = static_cast<wl_compositor*>(
        wl_registry_bind(registry, name, &wl_compositor_interface, 1));
  } else if (strcmp(interface, "wl_output") == 0) {
    WaylandScreen* screen = new WaylandScreen(disp, name);
    disp->screen_list_.push_back(screen);
    // (kalyan) Support extended output.
    disp->primary_screen_ = disp->screen_list_.front();
  } else if (strcmp(interface, "wl_seat") == 0) {
    WaylandInputDevice *input_device = new WaylandInputDevice(disp, name);
    disp->input_list_.push_back(input_device);
    disp->primary_input_device_ = disp->input_list_.front();
  } else if (strcmp(interface, "wl_shell") == 0) {
    disp->shell_ = static_cast<wl_shell*>(
        wl_registry_bind(registry, name, &wl_shell_interface, 1));
  } else if (strcmp(interface, "wl_shm") == 0) {
    disp->shm_ = static_cast<wl_shm*>(
        wl_registry_bind(registry, name, &wl_shm_interface, 1));
  }
}

}  // namespace ui
