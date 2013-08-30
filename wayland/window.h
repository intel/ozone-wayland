// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Copyright (C) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_WINDOW_H_
#define OZONE_WAYLAND_WINDOW_H_

#include <stdint.h>

#include "base/basictypes.h"
#include "ui/gfx/point.h"
#include "ui/gfx/rect.h"
#include "ozone/wayland/display.h"
#include <vector>

struct wl_surface;
struct wl_shell_surface;
struct wl_egl_window;

namespace ui {

class WaylandDisplay;
class WaylandSurface;

// WaylandWindow wraps a wl_surface and some basic operations for the surface.
class WaylandWindow {
 public:
  typedef std::vector<WaylandWindow*> Windows;

  // Creates a toplevel window.
  WaylandWindow();

  ~WaylandWindow();

  bool IsVisible() const;

  // Sets the window to fullscreen if |fullscreen| is true. Otherwise it sets
  // it as a normal window.
  void set_fullscreen(bool fullscreen) { fullscreen_ = fullscreen; }
  bool fullscreen() const { return fullscreen_; }

  void SetUserData(void *user_data) { user_data_ = user_data; }
  void* GetUserData() const { return user_data_; }

  // Returns a pointer to the parent window. NULL is this window doesn't have
  // a parent.
  WaylandWindow* GetToplevelWindow() { return this; }
  WaylandWindow* GetParentWindow() const { return parent_window_; }
  void SetParentWindow(WaylandWindow* parent_window);

  // Returns the pointer to the surface associated with the window.
  // The WaylandWindow object owns the pointer.
  WaylandSurface* surface() const { return surface_; }
  wl_shell_surface* shell_surface() const { return shell_surface_; }
  wl_egl_window* egl_window() const { return window_; }

  void SetBounds(const gfx::Rect& new_bounds);
  gfx::Rect GetBounds() const;
  void GetResizeDelta(int &x, int &y);

  void ScheduleResize(int32_t width, int32_t height);
  void SchedulePaintInRect(const gfx::Rect& rect);
  void ScheduleFlush();

  virtual void OnResize();

  const Windows& GetChildren() const { return children_; }
  void AddChild(WaylandWindow* child);
  void RemoveChild(WaylandWindow* child);

  void Show();
  void Hide();
  void SetType();

  static void HandleConfigure(void *data, struct wl_shell_surface *shell_surface,
      uint32_t edges, int32_t width, int32_t height);
  static void HandlePopupDone(void *data, struct wl_shell_surface *shell_surface);
  static void HandlePing(void *data, struct wl_shell_surface *shell_surface, uint32_t serial);

 private:
  static void FreeSurface(void *data, wl_callback *callback, uint32_t time);

  enum WindowType{
    TYPE_TOPLEVEL,
    TYPE_FULLSCREEN,
    TYPE_TRANSIENT,
    TYPE_MENU,
    TYPE_CUSTOM
  };

  enum WindowLocation {
    WINDOW_INTERIOR = 0,
    WINDOW_RESIZING_TOP = 1,
    WINDOW_RESIZING_BOTTOM = 2,
    WINDOW_RESIZING_LEFT = 4,
    WINDOW_RESIZING_TOP_LEFT = 5,
    WINDOW_RESIZING_BOTTOM_LEFT = 6,
    WINDOW_RESIZING_RIGHT = 8,
    WINDOW_RESIZING_TOP_RIGHT = 9,
    WINDOW_RESIZING_BOTTOM_RIGHT = 10,
    WINDOW_RESIZING_MASK = 15,
    WINDOW_EXTERIOR = 16,
    WINDOW_TITLEBAR = 17,
    WINDOW_CLIENT_AREA = 18,
  };

  // When creating a transient window, |parent_window_| is set to point to the
  // parent of this window. We will then use |parent_window_| to align this
  // window at the specified offset in |relative_position_|.
  // |parent_window_| is not owned by this window.
  WaylandWindow* parent_window_;

  // Position relative to parent window. This is only used by
  // a transient window.
  gfx::Point relative_position_;

  // The native wayland surface associated with this window.
  WaylandSurface* surface_;
  wl_shell_surface* shell_surface_;

  // Whether the window is in fullscreen mode.
  bool fullscreen_;

  int resize_edges_;
  gfx::Rect allocation_;
  gfx::Rect saved_allocation_;
  gfx::Rect server_allocation_;
  gfx::Rect pending_allocation_;
  WindowType type_;
  bool resize_scheduled_;
  wl_egl_window *window_;

  // Child windows. Topmost is last.
  Windows children_;

  void *user_data_;

  DISALLOW_COPY_AND_ASSIGN(WaylandWindow);
};

}  // namespace ui

#endif  // OZONE_WAYLAND_WINDOW_H_
