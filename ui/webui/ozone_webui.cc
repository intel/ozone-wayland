// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/webui/ozone_webui.h"

#include <set>

#include "base/command_line.h"
#include "base/debug/leak_annotations.h"
#include "base/environment.h"
#include "base/i18n/rtl.h"
#include "base/logging.h"
#include "base/nix/mime_util_xdg.h"
#include "base/stl_util.h"
#include "base/strings/stringprintf.h"
#include "ozone/ui/webui/select_file_dialog_impl_webui.h"

namespace views {

OzoneWebUI::OzoneWebUI() {
}

OzoneWebUI::~OzoneWebUI() {
}

void OzoneWebUI::Initialize() {
}

ui::SelectFileDialog* OzoneWebUI::CreateSelectFileDialog(
    ui::SelectFileDialog::Listener* listener,
    ui::SelectFilePolicy* policy) const {
  return ui::SelectFileDialogImplWebUI::Create(listener, policy);
}

scoped_ptr<ui::LinuxInputMethodContext> OzoneWebUI::CreateInputMethodContext(
      ui::LinuxInputMethodContextDelegate* delegate) const {
  return scoped_ptr<ui::LinuxInputMethodContext>();
}

gfx::FontRenderParams OzoneWebUI::GetDefaultFontRenderParams() const {
  NOTIMPLEMENTED();
  return params_;
}

scoped_ptr<gfx::ScopedPangoFontDescription>
  OzoneWebUI::GetDefaultPangoFontDescription() const {
  NOTIMPLEMENTED();
  return scoped_ptr<gfx::ScopedPangoFontDescription>();
}

double OzoneWebUI::GetFontDPI() const {
  NOTIMPLEMENTED();
  return 96.0;
}

gfx::Image OzoneWebUI::GetThemeImageNamed(int id) const {
  return gfx::Image(); 
}

bool OzoneWebUI::GetColor(int id, SkColor* color) const {
  return false;
}

bool OzoneWebUI::HasCustomImage(int id) const { 
  return false;
}

SkColor OzoneWebUI::GetFocusRingColor() const {
  return SK_ColorBLACK;
}

SkColor OzoneWebUI::GetThumbActiveColor() const {
  return SK_ColorBLACK;
}

SkColor OzoneWebUI::GetThumbInactiveColor() const {
  return SK_ColorBLACK;
}

SkColor OzoneWebUI::GetTrackColor() const {
  return SK_ColorBLACK;
}

SkColor OzoneWebUI::GetActiveSelectionBgColor() const {
  return SK_ColorBLACK;
}

SkColor OzoneWebUI::GetActiveSelectionFgColor() const {
  return SK_ColorBLACK;
}

SkColor OzoneWebUI::GetInactiveSelectionBgColor() const {
  return SK_ColorBLACK;
}

SkColor OzoneWebUI::GetInactiveSelectionFgColor() const {
  return SK_ColorBLACK;
}

double OzoneWebUI::GetCursorBlinkInterval() const {
  return 1.0;
}

ui::NativeTheme* OzoneWebUI::GetNativeTheme(aura::Window* window) const {
  return 0;
}

void OzoneWebUI::SetNativeThemeOverride(const NativeThemeGetter& callback) {
}

bool OzoneWebUI::GetDefaultUsesSystemTheme() const {
  return false;
}

void OzoneWebUI::SetDownloadCount(int count) const {
}

void OzoneWebUI::SetProgressFraction(float percentage) const {
}

bool OzoneWebUI::IsStatusIconSupported() const {
  return false;
}

scoped_ptr<StatusIconLinux> OzoneWebUI::CreateLinuxStatusIcon(
  const gfx::ImageSkia& image,
  const base::string16& tool_tip) const {
  return scoped_ptr<views::StatusIconLinux>();
}

gfx::Image OzoneWebUI::GetIconForContentType(
  const std::string& content_type, int size) const {
  return gfx::Image();
}

scoped_ptr<Border> OzoneWebUI::CreateNativeBorder(
  views::LabelButton* owning_button,
  scoped_ptr<views::LabelButtonBorder> border){
  return  border.PassAs<views::Border>();
}

void OzoneWebUI::AddWindowButtonOrderObserver(
  WindowButtonOrderObserver* observer) {
}

void OzoneWebUI::RemoveWindowButtonOrderObserver(
  WindowButtonOrderObserver* observer) {
}

bool OzoneWebUI::UnityIsRunning() {
  return 0;
}

LinuxUI::NonClientMiddleClickAction OzoneWebUI::GetNonClientMiddleClickAction() {
  return MIDDLE_CLICK_ACTION_NONE; 
}

void OzoneWebUI::NotifyWindowManagerStartupComplete() {
}

bool OzoneWebUI::MatchEvent(const ui::Event& event,
  std::vector<TextEditCommandAuraLinux>* commands) {
  return false;
}

}  // namespace views

views::LinuxUI* BuildWebUI() {
  return new views::OzoneWebUI;
}