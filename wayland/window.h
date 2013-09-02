// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_WINDOW_H_
#define OZONE_WAYLAND_WINDOW_H_

#include "ui/gfx/rect.h"
#include "ozone/wayland/display.h"

#include <stdint.h>
#include <vector>

namespace ui {

class WaylandDisplay;
class WaylandShellSurface;
class EGLWindow;
struct wl_egl_window;

class WaylandWindow {
 public:
  enum State {
    Default = 0x00,
    Visible = 0x01, // Whether the window is visible.
    Focus = 0x02, // Whether the window has current focus.
    FullScreen = 0x04,  // Whether the window is in fullscreen mode.
    Maximized = 0x08, // Window is maximized,
    Minimized = 0x10, // Window is minimized.
    Normal = 0x20, // Window is in Normal Mode.
    Activated = 0x40, // Window is Active.
    PendingResize = 0x80 // Pending Resize.
  };

  enum Shell {
    TOPLEVEL,
    FULLSCREEN,
    TRANSIENT,
    MENU,
    CUSTOM
  };

  typedef std::vector<WaylandWindow*> Windows;
  typedef unsigned WindowState;
  typedef unsigned ShellType;

  // Creates a toplevel window.
  WaylandWindow();
  ~WaylandWindow();

  WindowState State() const { return state_; }
  void SetShellType(ShellType type);

  // Sets the window to fullscreen if |fullscreen| is true. Otherwise it sets
  // it as a normal window.
  void SetFullScreen(bool fullscreen);
  void ToggleFullScreen() { SetFullScreen(!(state_ & FullScreen)); }

  void OnShow();
  void OnHide();

  void OnActivate();
  void OnDeActivate();

  void OnMaximize();
  void OnMinimize();
  void OnRestore();
  void SetFocus(bool focus);
  void SetWindowTitle(const char *title);

  void SetUserData(void *user_data) { user_data_ = user_data; }
  void* GetUserData() const { return user_data_; }

  // Returns a pointer to the parent window. NULL is this window doesn't have
  // a parent.
  WaylandWindow* GetToplevelWindow() { return this; }
  WaylandWindow* GetParentWindow() const { return parent_window_; }
  void SetParentWindow(WaylandWindow* parent_window);
  void RealizeAcceleratedWidget();

  // Returns the pointer to the surface associated with the window.
  // The WaylandWindow object owns the pointer.
  wl_egl_window* egl_window() const;
  struct wl_surface* wlSurface() const;

  void SetBounds(const gfx::Rect& new_bounds);
  gfx::Rect GetBounds() const { return allocation_; }
  void GetResizeDelta(int &x, int &y);

  const Windows& GetChildren() const { return children_; }
  void AddChild(WaylandWindow* child);
  void RemoveChild(WaylandWindow* child);

  void HandleConfigure(uint32_t edges, int32_t width, int32_t height);

 private:
  void UpdateWindowType();
  void HandleResize(int32_t width, int32_t height);
  void Resize();

  // When creating a transient window, |parent_window_| is set to point to the
  // parent of this window. We will then use |parent_window_| to align this
  // window at the specified offset.
  // |parent_window_| is not owned by this window.
  WaylandWindow* parent_window_;
  WaylandShellSurface* shell_surface_;
  EGLWindow* window_;
  void* user_data_;

  gfx::Rect allocation_;
  gfx::Rect saved_allocation_;
  gfx::Rect server_allocation_;
  int resize_edges_;
  WindowState state_;
  ShellType type_;

  // Child windows. Topmost is last.
  Windows children_;
  DISALLOW_COPY_AND_ASSIGN(WaylandWindow);
};

}  // namespace ui

#endif  // OZONE_WAYLAND_WINDOW_H_
