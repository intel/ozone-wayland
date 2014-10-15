// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/input_device.h"

#include "base/logging.h"
#include "ozone/wayland/display.h"
#include "ozone/wayland/input/cursor.h"
#include "ozone/wayland/input/keyboard.h"
#include "ozone/wayland/input/pointer.h"
#include "ozone/wayland/input/text_input.h"
#include "ozone/wayland/input/touchscreen.h"

namespace ozonewayland {

const int kCursorNull = 0;
const int kCursorPointer = 1;
const int kCursorCross = 2;
const int kCursorHand = 3;
const int kCursorIBeam = 4;
const int kCursorWait = 5;
const int kCursorHelp = 6;
const int kCursorEastResize = 7;
const int kCursorNorthResize = 8;
const int kCursorNorthEastResize = 9;
const int kCursorNorthWestResize = 10;
const int kCursorSouthResize = 11;
const int kCursorSouthEastResize = 12;
const int kCursorSouthWestResize = 13;
const int kCursorWestResize = 14;
const int kCursorNorthSouthResize = 15;
const int kCursorEastWestResize = 16;
const int kCursorNorthEastSouthWestResize = 17;
const int kCursorNorthWestSouthEastResize = 18;
const int kCursorColumnResize = 19;
const int kCursorRowResize = 20;
const int kCursorMiddlePanning = 21;
const int kCursorEastPanning = 22;
const int kCursorNorthPanning = 23;
const int kCursorNorthEastPanning = 24;
const int kCursorNorthWestPanning = 25;
const int kCursorSouthPanning = 26;
const int kCursorSouthEastPanning = 27;
const int kCursorSouthWestPanning = 28;
const int kCursorWestPanning = 29;
const int kCursorMove = 30;
const int kCursorVerticalText = 31;
const int kCursorCell = 32;
const int kCursorContextMenu = 33;
const int kCursorAlias = 34;
const int kCursorProgress = 35;
const int kCursorNoDrop = 36;
const int kCursorCopy = 37;
const int kCursorNone = 38;
const int kCursorNotAllowed = 39;
const int kCursorZoomIn = 40;
const int kCursorZoomOut = 41;
const int kCursorGrab = 42;
const int kCursorGrabbing = 43;
const int kCursorCustom = 44;

// Returns Wayland font cursor shape from an Aura cursor.
WaylandCursor::CursorType CursorShapeFromNative(unsigned cursor_type) {
  switch (cursor_type) {
    case kCursorMiddlePanning:
    case kCursorMove:
      return WaylandCursor::CURSOR_FLEUR;
    case kCursorEastPanning:
      return WaylandCursor::CURSOR_RIGHT;
    case kCursorNorthPanning:
      return WaylandCursor::CURSOR_UP_ARROW;
    case kCursorNorthEastPanning:
      return WaylandCursor::CURSOR_TOP_RIGHT;
    case kCursorNorthWestPanning:
      return WaylandCursor::CURSOR_TOP_LEFT;
    case kCursorSouthPanning:
      return WaylandCursor::CURSOR_BOTTOM;
    case kCursorSouthEastPanning:
    case kCursorSouthEastResize:
      return WaylandCursor::CURSOR_BOTTOM_RIGHT;
    case kCursorSouthWestPanning:
    case kCursorSouthWestResize:
      return WaylandCursor::CURSOR_BOTTOM_LEFT;
    case kCursorWestPanning:
      return WaylandCursor::CURSOR_LEFT_ARROW;
    case kCursorNone:
    case kCursorNull:
    case kCursorPointer:
    // TODO(kalyan): Choose right cursor types.
    case kCursorGrab:
    case kCursorGrabbing:
      return WaylandCursor::CURSOR_LEFT_PTR;
    case kCursorCross:
      return WaylandCursor::CURSOR_CROSS;
    case kCursorHand:
      return WaylandCursor::CURSOR_HAND1;
    case kCursorIBeam:
      return WaylandCursor::CURSOR_IBEAM;
    case kCursorProgress:
      return WaylandCursor::CURSOR_WATCH;
    case kCursorWait:
      return WaylandCursor::CURSOR_WAIT;
    case kCursorHelp:
      return WaylandCursor::CURSOR_QUESTION_ARROW;
    case kCursorEastResize:
      return WaylandCursor::CURSOR_RIGHT;
    case kCursorNorthResize:
      return WaylandCursor::CURSOR_TOP;
    case kCursorNorthEastResize:
      return WaylandCursor::CURSOR_TOP_RIGHT;
    case kCursorNorthWestResize:
      return WaylandCursor::CURSOR_TOP_LEFT_ARROW;
    case kCursorSouthResize:
      return WaylandCursor::CURSOR_BOTTOM;
    case kCursorWestResize:
      return WaylandCursor::CURSOR_LEFT;
    case kCursorNorthSouthResize:
    case kCursorRowResize:
      return WaylandCursor::CURSOR_V_DOUBLE_ARROW;
    case kCursorEastWestResize:
    case kCursorColumnResize:
      return WaylandCursor::CURSOR_H_DOUBLE_ARROW;
    case kCursorCustom:
      NOTREACHED();
      return WaylandCursor::CURSOR_DEFAULT;
  }
  NOTREACHED() << "Case not handled for " << cursor_type;
  return WaylandCursor::CURSOR_LEFT_PTR;
}

WaylandInputDevice::WaylandInputDevice(WaylandDisplay* display,
                                       uint32_t id)
    : focused_window_handle_(0),
      grab_window_handle_(0),
      grab_button_(0),
      input_seat_(NULL),
      input_keyboard_(NULL),
      input_pointer_(NULL),
      input_touch_(NULL),
      text_input_(NULL) {
  ui::IMEStateChangeHandler::SetInstance(this);
  static const struct wl_seat_listener kInputSeatListener = {
    WaylandInputDevice::OnSeatCapabilities,
  };

  input_seat_ = static_cast<wl_seat*>(
      wl_registry_bind(display->registry(), id, &wl_seat_interface, 1));
  DCHECK(input_seat_);
  wl_seat_add_listener(input_seat_, &kInputSeatListener, this);
  wl_seat_set_user_data(input_seat_, this);
  text_input_ = new WaylandTextInput(this);
}

WaylandInputDevice::~WaylandInputDevice() {
  delete input_keyboard_;
  delete input_pointer_;
  delete text_input_;
  if (input_touch_ != NULL) {
    delete input_touch_;
  }
  wl_seat_destroy(input_seat_);
}

void WaylandInputDevice::OnSeatCapabilities(void *data,
                                            wl_seat *seat,
                                            uint32_t caps) {
  WaylandInputDevice* device = static_cast<WaylandInputDevice*>(data);
  if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !device->input_keyboard_) {
    device->input_keyboard_ = new WaylandKeyboard();
    device->input_keyboard_->OnSeatCapabilities(seat, caps);
  } else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && device->input_keyboard_) {
    device->input_keyboard_->OnSeatCapabilities(seat, caps);
    delete device->input_keyboard_;
    device->input_keyboard_ = NULL;
  }

  if ((caps & WL_SEAT_CAPABILITY_POINTER) && !device->input_pointer_) {
    device->input_pointer_ = new WaylandPointer();
    device->input_pointer_->OnSeatCapabilities(seat, caps);
  } else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && device->input_pointer_) {
    device->input_pointer_->OnSeatCapabilities(seat, caps);
    delete device->input_pointer_;
    device->input_pointer_ = NULL;
  }

  if ((caps & WL_SEAT_CAPABILITY_TOUCH) && !device->input_touch_) {
    device->input_touch_ = new WaylandTouchscreen();
    device->input_touch_->OnSeatCapabilities(seat, caps);
  } else if (!(caps & WL_SEAT_CAPABILITY_TOUCH) && device->input_touch_) {
    delete device->input_touch_;
    device->input_touch_ = NULL;
  }
}

void WaylandInputDevice::SetFocusWindowHandle(unsigned windowhandle) {
  focused_window_handle_ = windowhandle;
  WaylandWindow* window = NULL;
  if (windowhandle)
    window = WaylandDisplay::GetInstance()->GetWindow(windowhandle);
  text_input_->SetActiveWindow(window);
}

void WaylandInputDevice::SetGrabWindowHandle(unsigned windowhandle,
                                             uint32_t button) {
  grab_window_handle_ = windowhandle;
  grab_button_ = button;
}

void WaylandInputDevice::SetCursorType(int cursor_type) {
  if (!input_pointer_) {
    LOG(WARNING) << "Tried to change cursor without input configured";
    return;
  }
  input_pointer_->Cursor()->Update(CursorShapeFromNative(cursor_type),
                                   WaylandDisplay::GetInstance()->GetSerial());
}

void WaylandInputDevice::ResetIme() {
  text_input_->ResetIme();
}

void WaylandInputDevice::ImeCaretBoundsChanged(gfx::Rect rect) {
  NOTIMPLEMENTED();
}

void WaylandInputDevice::ShowInputPanel() {
  text_input_->ShowInputPanel(input_seat_);
}

void WaylandInputDevice::HideInputPanel() {
  text_input_->HideInputPanel(input_seat_);
}

}  // namespace ozonewayland
