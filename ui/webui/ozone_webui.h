// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_OZONE_WEB_UI_H_
#define CHROME_BROWSER_UI_OZONE_WEB_UI_H_

#include <map>
#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/observer_list.h"
#include "ui/base/ime/linux/linux_input_method_context.h"
#include "ui/events/linux/text_edit_key_bindings_delegate_auralinux.h"
#include "ui/gfx/color_utils.h"
#include "ui/gfx/font_render_params.h"
#include "ui/gfx/geometry/insets.h"
#include "ui/gfx/image/image.h"
#include "ui/gfx/pango_util.h"
#include "ui/views/border.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/controls/button/label_button_border.h"
#include "ui/views/linux_ui/linux_ui.h"
#include "ui/views/window/frame_buttons.h"

class SkBitmap;

namespace gfx {
class Image;
}

using ui::TextEditCommandAuraLinux;

namespace views {
class Border;
class LabelButton;
class View;
class NativeThemeChangeObserver;
class WindowButtonOrderObserver;
// Interface to Wayland desktop features.
//
class OzoneWebUI : public views::LinuxUI {
 public:
  OzoneWebUI();
  virtual ~OzoneWebUI();

  // ui::LinuxInputMethodContextFactory:
  virtual scoped_ptr<ui::LinuxInputMethodContext> CreateInputMethodContext(
      ui::LinuxInputMethodContextDelegate* delegate) const OVERRIDE;

  // gfx::LinuxFontDelegate:
  virtual gfx::FontRenderParams GetDefaultFontRenderParams() const OVERRIDE; 
  virtual scoped_ptr<gfx::ScopedPangoFontDescription>
      GetDefaultPangoFontDescription() const OVERRIDE;
  virtual double GetFontDPI() const OVERRIDE;

  // ui::LinuxShellDialog:
  virtual ui::SelectFileDialog* CreateSelectFileDialog(
      ui::SelectFileDialog::Listener* listener,
      ui::SelectFilePolicy* policy) const OVERRIDE;

  // ui::LinuxUI:
  virtual void Initialize() OVERRIDE;

  // These methods are not needed
  virtual gfx::Image GetThemeImageNamed(int id) const OVERRIDE;
  virtual bool GetColor(int id, SkColor* color) const OVERRIDE;
  virtual bool HasCustomImage(int id) const OVERRIDE;
  virtual SkColor GetFocusRingColor() const OVERRIDE;
  virtual SkColor GetThumbActiveColor() const OVERRIDE;
  virtual SkColor GetThumbInactiveColor() const OVERRIDE;
  virtual SkColor GetTrackColor() const OVERRIDE;
  virtual SkColor GetActiveSelectionBgColor() const OVERRIDE;
  virtual SkColor GetActiveSelectionFgColor() const OVERRIDE;
  virtual SkColor GetInactiveSelectionBgColor() const OVERRIDE;
  virtual SkColor GetInactiveSelectionFgColor() const OVERRIDE;
  virtual double GetCursorBlinkInterval() const OVERRIDE;
  virtual ui::NativeTheme* GetNativeTheme(aura::Window* window) const OVERRIDE;
  virtual void SetNativeThemeOverride(const NativeThemeGetter& callback)
      OVERRIDE;
  virtual bool GetDefaultUsesSystemTheme() const OVERRIDE;
  virtual void SetDownloadCount(int count) const OVERRIDE;
  virtual void SetProgressFraction(float percentage) const OVERRIDE;
  virtual bool IsStatusIconSupported() const OVERRIDE;
  virtual scoped_ptr<StatusIconLinux> CreateLinuxStatusIcon(
      const gfx::ImageSkia& image,
      const base::string16& tool_tip) const OVERRIDE; 
  virtual gfx::Image GetIconForContentType(
      const std::string& content_type, int size) const OVERRIDE;
  virtual scoped_ptr<Border> CreateNativeBorder(
      views::LabelButton* owning_button,
      scoped_ptr<views::LabelButtonBorder> border) OVERRIDE;
  virtual void AddWindowButtonOrderObserver(
      WindowButtonOrderObserver* observer) OVERRIDE;
  virtual void RemoveWindowButtonOrderObserver(
      WindowButtonOrderObserver* observer) OVERRIDE;
  virtual bool UnityIsRunning() OVERRIDE;
  virtual NonClientMiddleClickAction GetNonClientMiddleClickAction() OVERRIDE; 
  virtual void NotifyWindowManagerStartupComplete() OVERRIDE;

  virtual bool MatchEvent(const ui::Event& event,
     std::vector<TextEditCommandAuraLinux>* commands) OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(OzoneWebUI);
  gfx::FontRenderParams params_;
};

}  // namespace views

views::LinuxUI* BuildWebUI();

#endif  // CHROME_BROWSER_UI_OZONE_WEB_UI_H_
