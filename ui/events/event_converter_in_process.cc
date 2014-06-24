// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/events/event_converter_in_process.h"

#include "base/bind.h"
#include "ozone/ui/events/output_change_observer.h"
#include "ozone/ui/events/window_change_observer.h"

namespace ui {

EventConverterInProcess::EventConverterInProcess()
    : EventConverterOzoneWayland(),
      observer_(NULL),
      output_observer_(NULL) {
}

EventConverterInProcess::~EventConverterInProcess() {
}

void EventConverterInProcess::MotionNotify(float x, float y) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyMotion, this, x, y));
}

void EventConverterInProcess::ButtonNotify(unsigned handle,
                                           ui::EventType type,
                                           ui::EventFlags flags,
                                           float x,
                                           float y) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyButtonPress, this, handle, type,
          flags, x, y));
}

void EventConverterInProcess::AxisNotify(float x,
                                         float y,
                                         int xoffset,
                                         int yoffset) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyAxis, this, x, y, xoffset, yoffset));
}

void EventConverterInProcess::PointerEnter(unsigned handle,
                                           float x,
                                           float y) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyPointerEnter, this, handle, x, y));
}

void EventConverterInProcess::PointerLeave(unsigned handle,
                                           float x,
                                           float y) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyPointerLeave, this, handle, x, y));
}

void EventConverterInProcess::KeyNotify(ui::EventType type,
                                        unsigned code,
                                        unsigned modifiers) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyKeyEvent, this, type,
          ui::KeyboardCodeFromNativeKeysym(code),
              ui::CharacterCodeFromNativeKeySym(code, modifiers), modifiers));
}

void EventConverterInProcess::TouchNotify(ui::EventType type,
                                          float x,
                                          float y,
                                          int32_t touch_id,
                                          uint32_t time_stamp) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyTouchEvent, this, type, x, y, touch_id,
          time_stamp));
}

void EventConverterInProcess::CloseWidget(unsigned handle) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyCloseWidget, this, handle));
}

void EventConverterInProcess::OutputSizeChanged(unsigned width,
                                                unsigned height) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyOutputSizeChanged, this, width, height));
}

void EventConverterInProcess::WindowResized(unsigned handle,
                                            unsigned width,
                                            unsigned height) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyWindowResized, this, handle, width,
          height));
}

void EventConverterInProcess::Commit(unsigned handle, const std::string& text) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyCommit, this, handle, text));
}

void EventConverterInProcess::PreeditChanged(unsigned handle,
                                             const std::string& text,
                                             const std::string& commit) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyPreeditChanged, this, handle, text,
      commit));
}

void EventConverterInProcess::PreeditEnd() {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
        &EventConverterInProcess::NotifyPreeditEnd, this));
}

void EventConverterInProcess::PreeditStart() {
    ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
        &EventConverterInProcess::NotifyPreeditStart, this));
}

void EventConverterInProcess::SetWindowChangeObserver(
    ui::WindowChangeObserver* observer) {
  observer_ = observer;
}

void EventConverterInProcess::SetOutputChangeObserver(
    ui::OutputChangeObserver* observer) {
  output_observer_ = observer;
}

void EventConverterInProcess::OnDispatcherListChanged() {
  if (!loop_)
    loop_ = base::MessageLoop::current();
}

void EventConverterInProcess::NotifyMotion(EventConverterInProcess* data,
                                           float x,
                                           float y) {
  gfx::Point position(x, y);
  ui::MouseEvent mouseev(ui::ET_MOUSE_MOVED,
                         position,
                         position,
                         0,
                         0);
  data->DispatchEvent(&mouseev);
}

void EventConverterInProcess::NotifyButtonPress(EventConverterInProcess* data,
                                                unsigned handle,
                                                ui::EventType type,
                                                ui::EventFlags flags,
                                                float x,
                                                float y) {
    gfx::Point position(x, y);
    ui::MouseEvent mouseev(type,
                           position,
                           position,
                           flags,
                           1);
    data->DispatchEvent(&mouseev);

    if (type == ui::ET_MOUSE_RELEASED && data->observer_)
      data->observer_->OnWindowFocused(handle);
}

void EventConverterInProcess::NotifyAxis(EventConverterInProcess* data,
                                         float x,
                                         float y,
                                         int xoffset,
                                         int yoffset) {
  gfx::Point position(x, y);
  ui::MouseEvent mouseev(ui::ET_MOUSEWHEEL, position, position, 0, 0);
  ui::MouseWheelEvent wheelev(mouseev, xoffset, yoffset);

  data->DispatchEvent(&wheelev);
}

void EventConverterInProcess::NotifyPointerEnter(
    EventConverterInProcess* data, unsigned handle, float x, float y) {
  if (data->observer_)
    data->observer_->OnWindowEnter(handle);

  gfx::Point position(x, y);
  ui::MouseEvent mouseev(ui::ET_MOUSE_ENTERED, position, position, 0, 0);
  data->DispatchEvent(&mouseev);
}

void EventConverterInProcess::NotifyPointerLeave(
    EventConverterInProcess* data, unsigned handle, float x, float y) {
  if (data->observer_)
    data->observer_->OnWindowLeave(handle);

  gfx::Point position(x, y);
  ui::MouseEvent mouseev(ui::ET_MOUSE_EXITED, position, position, 0, 0);
  data->DispatchEvent(&mouseev);
}

void EventConverterInProcess::NotifyKeyEvent(EventConverterInProcess* data,
                                             ui::EventType type,
                                             ui::KeyboardCode code,
                                             uint16 CharacterCodeFromNativeKey,
                                             unsigned modifiers) {
  ui::KeyEvent keyev(type, code, modifiers, false);
  keyev.set_character(CharacterCodeFromNativeKey);
  data->DispatchEvent(&keyev);
}

void EventConverterInProcess::NotifyTouchEvent(EventConverterInProcess* data,
                                               ui::EventType type,
                                               float x,
                                               float y,
                                               int32_t touch_id,
                                               uint32_t time_stamp) {
  gfx::Point position(x, y);
  base::TimeDelta time_delta = base::TimeDelta::FromMilliseconds(time_stamp);
  ui::TouchEvent touchev(type, position, touch_id, time_delta);
  data->DispatchEvent(&touchev);
}

void EventConverterInProcess::NotifyCloseWidget(
    EventConverterInProcess* data, unsigned handle) {
  if (data->observer_)
    data->observer_->OnWindowClose(handle);
}

void
EventConverterInProcess::NotifyOutputSizeChanged(EventConverterInProcess* data,
                                                 unsigned width,
                                                 unsigned height) {
  if (data->output_observer_)
    data->output_observer_->OnOutputSizeChanged(width, height);
}

void
EventConverterInProcess::NotifyWindowResized(EventConverterInProcess* data,
                                             unsigned handle,
                                             unsigned width,
                                             unsigned height) {
  if (data->observer_)
    data->observer_->OnWindowResized(handle, width, height);
}

void
EventConverterInProcess::NotifyCommit(EventConverterInProcess* data,
                                      unsigned handle,
                                      const std::string& text) {
  if (data->observer_)
    data->observer_->OnCommit(handle, text);
}

void
EventConverterInProcess::NotifyPreeditChanged(EventConverterInProcess* data,
                                              unsigned handle,
                                              const std::string& text,
                                              const std::string& commit) {
  if (data->observer_)
    data->observer_->OnPreeditChanged(handle, text, commit);
}

void
EventConverterInProcess::NotifyPreeditEnd(EventConverterInProcess* data) {
}

void
EventConverterInProcess::NotifyPreeditStart(EventConverterInProcess* data) {
}

}  // namespace ui
