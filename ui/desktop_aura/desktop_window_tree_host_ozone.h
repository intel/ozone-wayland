// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_WINDOW_TREE_HOST_OZONE_H_
#define UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_WINDOW_TREE_HOST_OZONE_H_

#include <set>
#include <string>
#include <vector>

#include "base/basictypes.h"
#include "ui/aura/window_tree_host.h"
#include "ui/events/event_source.h"
#include "ui/events/platform/platform_event_dispatcher.h"
#include "ui/platform_window/platform_window_delegate.h"
#include "ui/views/widget/desktop_aura/desktop_window_tree_host.h"

namespace ui {
class PlatformWindow;
}

namespace views {

namespace corewm {
class Tooltip;
}

class DesktopDragDropClientWayland;

class VIEWS_EXPORT DesktopWindowTreeHostOzone
    : public DesktopWindowTreeHost,
      public aura::WindowTreeHost,
      public ui::PlatformWindowDelegate,
      public ui::PlatformEventDispatcher,
      public ui::EventSource
{
 public:
  DesktopWindowTreeHostOzone(
      internal::NativeWidgetDelegate* native_widget_delegate,
      DesktopNativeWidgetAura* desktop_native_widget_aura);
  virtual ~DesktopWindowTreeHostOzone();

  // Accepts a opaque handle widget and returns associated aura::Window.
  static aura::Window* GetContentWindowForAcceleratedWidget(
      gfx::AcceleratedWidget widget);

  // Accepts a opaque handle widget and returns associated
  // DesktopWindowTreeHostOzone.
  static DesktopWindowTreeHostOzone* GetHostForAcceleratedWidget(
      gfx::AcceleratedWidget widget);

  // Get all open top-level windows. This includes windows that may not be
  // visible. This list is sorted in their stacking order, i.e. the first window
  // is the topmost window.
  static const std::vector<aura::Window*>& GetAllOpenWindows();

  // Deallocates the internal list of open windows.
  static void CleanUpWindowList();

  // Returns window bounds. This is used by Screen to determine if a point
  // belongs to a particular window.
  gfx::Rect GetBoundsInScreen() const;

 protected:
  // Overridden from DesktopWindowTreeHost:
  virtual void Init(
      aura::Window* content_window,
      const views::Widget::InitParams& params) override;
  virtual void OnNativeWidgetCreated(
      const views::Widget::InitParams& params) override;
  scoped_ptr<corewm::Tooltip> CreateTooltip() override;
  scoped_ptr<aura::client::DragDropClient> CreateDragDropClient(
      DesktopNativeCursorManager* cursor_manager) override;
  void Close() override;
  void CloseNow() override;
  virtual aura::WindowTreeHost* AsWindowTreeHost() override;
  virtual void ShowWindowWithState(ui::WindowShowState show_state) override;
  virtual void ShowMaximizedWithBounds(
      const gfx::Rect& restored_bounds) override;
  virtual bool IsVisible() const override;
  virtual void SetSize(const gfx::Size& size) override;
  virtual void StackAtTop() override;
  virtual void CenterWindow(const gfx::Size& size) override;
  virtual void GetWindowPlacement(
      gfx::Rect* bounds,
      ui::WindowShowState* show_state) const override;
  virtual gfx::Rect GetWindowBoundsInScreen() const override;
  virtual gfx::Rect GetClientAreaBoundsInScreen() const override;
  virtual gfx::Rect GetRestoredBounds() const override;
  virtual gfx::Rect GetWorkAreaBoundsInScreen() const override;
  virtual void SetShape(gfx::NativeRegion native_region) override;
  virtual void Activate() override;
  virtual void Deactivate() override;
  virtual bool IsActive() const override;
  virtual void Maximize() override;
  virtual void Minimize() override;
  virtual void Restore() override;
  virtual bool IsMaximized() const override;
  virtual bool IsMinimized() const override;
  virtual bool HasCapture() const override;
  virtual void SetAlwaysOnTop(bool always_on_top) override;
  virtual bool IsAlwaysOnTop() const override;
  virtual void SetVisibleOnAllWorkspaces(bool always_visible) override;
  virtual bool SetWindowTitle(const base::string16& title) override;
  virtual void ClearNativeFocus() override;
  virtual views::Widget::MoveLoopResult RunMoveLoop(
      const gfx::Vector2d& drag_offset,
      views::Widget::MoveLoopSource source,
      views::Widget::MoveLoopEscapeBehavior escape_behavior) override;
  virtual void EndMoveLoop() override;
  virtual void SetVisibilityChangedAnimationsEnabled(bool value) override;
  virtual bool ShouldUseNativeFrame() const override;
  virtual bool ShouldWindowContentsBeTransparent() const override;
  virtual void FrameTypeChanged() override;
  virtual void SetFullscreen(bool fullscreen) override;
  virtual bool IsFullscreen() const override;
  virtual void SetOpacity(unsigned char opacity) override;
  virtual void SetWindowIcons(const gfx::ImageSkia& window_icon,
                              const gfx::ImageSkia& app_icon) override;
  virtual void InitModalType(ui::ModalType modal_type) override;
  virtual void FlashFrame(bool flash_frame) override;
  virtual void OnRootViewLayout() override;
  virtual void OnNativeWidgetFocus() override;
  virtual void OnNativeWidgetBlur() override;
  virtual bool IsAnimatingClosed() const override;
  virtual bool IsTranslucentWindowOpacitySupported() const override;
  virtual void SizeConstraintsChanged() override;

  // Overridden from aura::WindowTreeHost:
  virtual ui::EventSource* GetEventSource() override;
  virtual gfx::AcceleratedWidget GetAcceleratedWidget() override;
  virtual void Show() override;
  virtual void Hide() override;
  virtual gfx::Rect GetBounds() const override;
  virtual void SetBounds(const gfx::Rect& bounds) override;
  virtual gfx::Point GetLocationOnNativeScreen() const override;
  virtual void SetCapture() override;
  virtual void ReleaseCapture() override;
  virtual void SetCursorNative(gfx::NativeCursor cursor) override;
  virtual void MoveCursorToNative(const gfx::Point& location) override;
  virtual void OnCursorVisibilityChangedNative(bool show) override;

  // Overridden from ui::PlatformWindowDelegate:
  virtual void OnBoundsChanged(const gfx::Rect&) override;
  virtual void OnDamageRect(const gfx::Rect& damaged_region) override;
  virtual void DispatchEvent(ui::Event* event) override;
  virtual void OnCloseRequest() override;
  virtual void OnClosed() override{}
  virtual void OnWindowStateChanged(ui::PlatformWindowState new_state) override;
  virtual void OnLostCapture() override;
  virtual void OnAcceleratedWidgetAvailable(
      gfx::AcceleratedWidget widget) override;
  virtual void OnActivationChanged(bool active) override;

  // Overridden frm ui::EventSource
  virtual ui::EventProcessor* GetEventProcessor() override;
  // ui::PlatformEventDispatcher:
  virtual bool CanDispatchEvent(const ui::PlatformEvent& event) override;
  virtual uint32_t DispatchEvent(const ui::PlatformEvent& event) override;

 private:
  enum {
    Uninitialized = 0x00,
    Visible = 0x01,  // Window is Visible.
    FullScreen = 0x02,  // Window is in fullscreen mode.
    Maximized = 0x04,  // Window is maximized,
    Minimized = 0x08,  // Window is minimized.
    Active = 0x10  // Window is Active.
  };

  typedef unsigned RootWindowState;

  // Initializes our Ozone surface to draw on. This method performs all
  // initialization related to talking to the Ozone server.
  void InitOzoneWindow(const views::Widget::InitParams& params);

  void Relayout();
  gfx::Size AdjustSize(const gfx::Size& requested_size);
  void DispatchMouseEvent(ui::MouseEvent* event);
  void ReleaseCaptureIfNeeded() const;
  void ShowWindow();

  static std::list<gfx::AcceleratedWidget>& open_windows();

  RootWindowState state_;

  // Original bounds of DRWH.
  gfx::Rect previous_bounds_;
  gfx::Rect previous_maximize_bounds_;
  gfx::AcceleratedWidget window_;
  base::string16 title_;

  base::WeakPtrFactory<DesktopWindowTreeHostOzone> close_widget_factory_;

  // Owned by DesktopNativeWidgetAura.
  DesktopDragDropClientWayland* drag_drop_client_;
  views::internal::NativeWidgetDelegate* native_widget_delegate_;
  aura::Window* content_window_;

  views::DesktopNativeWidgetAura* desktop_native_widget_aura_;
  // We can optionally have a parent which can order us to close, or own
  // children who we're responsible for closing when we CloseNow().
  DesktopWindowTreeHostOzone* window_parent_;
  std::set<DesktopWindowTreeHostOzone*> window_children_;

  // Platform-specific part of this DesktopWindowTreeHost.
  scoped_ptr<ui::PlatformWindow> platform_window_;

  // A list of all (top-level) windows that have been created but not yet
  // destroyed.
  static std::list<gfx::AcceleratedWidget>* open_windows_;
  // List of all open aura::Window.
  static std::vector<aura::Window*>* aura_windows_;
  static DesktopWindowTreeHostOzone* g_current_capture;
  static DesktopWindowTreeHostOzone* g_active_window;
  static DesktopWindowTreeHostOzone* g_current_dispatcher;
  DISALLOW_COPY_AND_ASSIGN(DesktopWindowTreeHostOzone);
};

}  // namespace views

#endif  // UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_WINDOW_TREE_HOST_OZONE_H_
