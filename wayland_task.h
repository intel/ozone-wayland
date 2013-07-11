// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_TASK_H_
#define OZONE_WAYLAND_TASK_H_

#include <stdint.h>

#include <list>

#include "base/basictypes.h"
#include "ui/gfx/point.h"
#include "ui/gfx/rect.h"

struct wl_compositor;
struct wl_display;
struct wl_shell;
struct wl_shm;
struct wl_surface;
struct wl_shell_surface;

namespace ui {

class WaylandDisplay;
class WaylandWindow;

class WaylandTask {
 public:
  WaylandTask(WaylandWindow* window);
  virtual ~WaylandTask();
  virtual void Run() = 0;
  WaylandWindow* GetWindow() { return window_; }

 protected:
  WaylandWindow *window_;

 private:
  DISALLOW_COPY_AND_ASSIGN(WaylandTask);
};

class WaylandResizeTask : public WaylandTask {
 public:
  WaylandResizeTask(WaylandWindow* window);
  virtual ~WaylandResizeTask();
  virtual void Run();

 private:
  DISALLOW_COPY_AND_ASSIGN(WaylandResizeTask);
};

class WaylandRedrawTask : public WaylandTask {
 public:
  WaylandRedrawTask(WaylandWindow* window);
  virtual ~WaylandRedrawTask();
  virtual void Run();

 private:
  DISALLOW_COPY_AND_ASSIGN(WaylandRedrawTask);
};

}  // namespace ui

#endif  // OZONE_WAYLAND_TASK_H_
