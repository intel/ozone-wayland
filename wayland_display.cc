// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland_display.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "base/message_loop/message_loop.h"
#include "ozone/wayland_input_device.h"
#include "ozone/wayland_screen.h"
#include "ozone/wayland_window.h"
#include "ozone/wayland_input_method_event_filter.h"

namespace ui {

WaylandDisplay* g_display = NULL;

WaylandDisplay* WaylandDisplay::GetDisplay()
{
  if (!g_display)
    g_display = WaylandDisplay::Connect(NULL);
  return g_display;
}

void WaylandDisplay::DestroyDisplay()
{
  if(g_display)
    delete g_display;
  g_display = NULL;
}

// static
WaylandDisplay* WaylandDisplay::Connect(char* name)
{
  static const struct wl_registry_listener registry_listener = {
    WaylandDisplay::DisplayHandleGlobal
  };

  WaylandDisplay* display = new WaylandDisplay(name);
  if (!display->display_) {
    delete display;
    return NULL;
  }

  wl_display_set_user_data(display->display_, display);

  display->registry_ =  wl_display_get_registry(display->display_);
  wl_registry_add_listener(display->registry_, &registry_listener, display);

  wl_display_dispatch(display->display_);

  display->CreateCursors();

  return display;
}

void WaylandDisplay::AddWindow(WaylandWindow* window)
{
  if(window)
    window_list_.push_back(window);
}

void WaylandDisplay::AddTask(WaylandTask* task)
{
  if(task)
    task_list_.push_back(task);
}

void WaylandDisplay::ProcessTasks()
{
  WaylandTask *task = NULL;

  while(!task_list_.empty())
  {
    task = task_list_.front();
    task->Run();
    task_list_.pop_front();
    delete task;
  }
}

void WaylandDisplay::RemoveWindow(WaylandWindow* window)
{
  if(!window)
    return;

  WaylandTask *task = NULL;

  for (std::list<WaylandTask*>::iterator i = task_list_.begin();
      i != task_list_.end(); ++i) {
    if((*i)->GetWindow() == window)
    {
      delete *i;
      i = task_list_.erase(i);
    }
  }

  for (std::list<WaylandWindow*>::iterator i = window_list_.begin();
      i != window_list_.end(); ++i) {
    if((*i) == window)
    {
      i = window_list_.erase(i);
      break;
    }
  }

  if(window_list_.size() < 1)
  {
    base::MessageLoop::current()->PostTask(FROM_HERE, base::MessageLoop::QuitClosure());
  }
}

bool WaylandDisplay::IsWindow(WaylandWindow* window)
{
  for (std::list<WaylandWindow*>::iterator i = window_list_.begin();
      i != window_list_.end(); ++i) {
    if((*i) == window)
    {
      return true;
    }
  }

  return false;
}

InputMethod* WaylandDisplay::GetInputMethod() const
{
  return input_method_filter_ ? input_method_filter_->GetInputMethod(): NULL;
}

// static
WaylandDisplay* WaylandDisplay::GetDisplay(wl_display* display)
{
  return static_cast<WaylandDisplay*>(wl_display_get_user_data(display));
}

WaylandDisplay::WaylandDisplay(char* name) : display_(NULL),
    cursor_theme_(NULL),
    cursors_(NULL),
    compositor_(NULL),
    shell_(NULL),
    shm_(NULL)
{
  display_ = wl_display_connect(name);
  input_method_filter_ = new WaylandInputMethodEventFilter;
}

WaylandDisplay::~WaylandDisplay()
{
  if (window_list_.size() > 0)
    fprintf(stderr, "warning: windows exist.\n");

  if (task_list_.size() > 0)
    fprintf(stderr, "warning: deferred tasks exist.\n");

  for (std::list<WaylandInputDevice*>::iterator i = input_list_.begin();
      i != input_list_.end(); ++i) {
    delete *i;
  }

  for (std::list<WaylandScreen*>::iterator i = screen_list_.begin();
      i != screen_list_.end(); ++i) {
    delete *i;
  }

  DestroyCursors();

  if (compositor_)
    wl_compositor_destroy(compositor_);
  if (shell_)
    wl_shell_destroy(shell_);
  if (shm_)
    wl_shm_destroy(shm_);
  if (display_)
    wl_display_disconnect(display_);

  delete input_method_filter_;
}

wl_surface* WaylandDisplay::CreateSurface()
{
  return wl_compositor_create_surface(compositor_);
}

std::list<WaylandScreen*> WaylandDisplay::GetScreenList() const {
  return screen_list_;
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
  } else if (strcmp(interface, "wl_seat") == 0) {
    WaylandInputDevice *input_device = new WaylandInputDevice(disp, name);
    disp->input_list_.push_back(input_device);
  } else if (strcmp(interface, "wl_shell") == 0) {
    disp->shell_ = static_cast<wl_shell*>(
        wl_registry_bind(registry, name, &wl_shell_interface, 1));
  } else if (strcmp(interface, "wl_shm") == 0) {
    disp->shm_ = static_cast<wl_shm*>(
        wl_registry_bind(registry, name, &wl_shm_interface, 1));
  }
}

static const char *cursor_names[] = {
  "bottom_left_corner",
  "bottom_right_corner",
  "bottom_side",
  "grabbing",
  "left_ptr",
  "left_side",
  "right_side",
  "top_left_corner",
  "top_right_corner",
  "top_side",
  "xterm",
  "hand1",
};

void WaylandDisplay::CreateCursors()
{
  unsigned int i, array_size = sizeof(cursor_names) / sizeof(cursor_names[0]);
  cursor_theme_ = wl_cursor_theme_load(NULL, 32, shm_);
  cursors_ = new wl_cursor*[array_size];
  memset(cursors_, 0, sizeof(wl_cursor*) * array_size);

  for(i = 0; i < array_size; i++)
    cursors_[i] = wl_cursor_theme_get_cursor(cursor_theme_, cursor_names[i]);
}

void WaylandDisplay::DestroyCursors()
{
  if(cursor_theme_)
  {
    wl_cursor_theme_destroy(cursor_theme_);
    cursor_theme_ = NULL;
  }

  if(cursors_)
  {
    delete[] cursors_;
    cursors_ = NULL;
  }
}

void WaylandDisplay::SetPointerImage(WaylandInputDevice *device, uint32_t time, int index)
{
  struct wl_buffer *buffer;
  struct wl_cursor *cursor;
  struct wl_cursor_image *image;

  if (index == device->GetCurrentPointerImage())
    return;

  cursor = cursors_[index];
  if (!cursor)
    return;

  image = cursor->images[0];
  buffer = wl_cursor_image_get_buffer(image);
  if (!buffer)
    return;

  device->SetCurrentPointerImage(index);
  // TODO:
  //  wl_pointer_set_cursor(device->GetPointer(), time, buffer,
  //                        image->hotspot_x, image->hotspot_y);
}

}  // namespace ui
