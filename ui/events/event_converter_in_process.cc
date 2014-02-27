// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/events/event_converter_in_process.h"

#include "base/bind.h"
#include "ozone/ui/events/keyboard_code_conversion_ozone.h"
#include "ozone/ui/events/output_change_observer.h"
#include "ozone/ui/events/window_change_observer.h"

namespace ozonewayland {

EventConverterInProcess::EventConverterInProcess()
    : EventConverterOzoneWayland(),
      observer_(NULL),
      output_observer_(NULL) {
}

EventConverterInProcess::~EventConverterInProcess() {
}

void EventConverterInProcess::MotionNotify(float x, float y) {
  gfx::Point position(x, y);
  scoped_ptr<ui::MouseEvent> mouseev(new ui::MouseEvent(ui::ET_MOUSE_MOVED,
                                                        position,
                                                        position,
                                                        0,
                                                        0));

  PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::DispatchEventHelper, base::Passed(
          mouseev.PassAs<ui::Event>())));
}

void EventConverterInProcess::ButtonNotify(unsigned handle,
                                           ui::EventType type,
                                           ui::EventFlags flags,
                                           float x,
                                           float y) {
  gfx::Point position(x, y);
  scoped_ptr<ui::MouseEvent> mouseev(new ui::MouseEvent(type,
                                                        position,
                                                        position,
                                                        flags,
                                                        1));
  PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::DispatchEventHelper, base::Passed(
          mouseev.PassAs<ui::Event>())));

  if (type == ui::ET_MOUSE_RELEASED)
    PostTaskOnMainLoop(base::Bind(
        &EventConverterInProcess::NotifyButtonPress, this, handle));
}

void EventConverterInProcess::AxisNotify(float x,
                                         float y,
                                         int xoffset,
                                         int yoffset) {
  gfx::Point position(x, y);
  ui::MouseEvent mouseev(ui::ET_MOUSEWHEEL, position, position, 0, 0);

  scoped_ptr<ui::MouseWheelEvent> wheelev(new ui::MouseWheelEvent(mouseev,
                                                                  xoffset,
                                                                  yoffset));

  PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::DispatchEventHelper, base::Passed(
          wheelev.PassAs<ui::Event>())));
}

void EventConverterInProcess::PointerEnter(unsigned handle,
                                           float x,
                                           float y) {
  gfx::Point position(x, y);
  scoped_ptr<ui::MouseEvent> mouseev(new ui::MouseEvent(ui::ET_MOUSE_ENTERED,
                                                        position,
                                                        position,
                                                        0,
                                                        0));
  PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyPointerEnter, this, handle));

  PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::DispatchEventHelper, base::Passed(
          mouseev.PassAs<ui::Event>())));
}

void EventConverterInProcess::PointerLeave(unsigned handle,
                                           float x,
                                           float y) {
  gfx::Point position(x, y);
  scoped_ptr<ui::MouseEvent> mouseev(new ui::MouseEvent(ui::ET_MOUSE_EXITED,
                                                        position,
                                                        position,
                                                        0,
                                                        0));

  PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyPointerLeave, this, handle));

  PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::DispatchEventHelper, base::Passed(
          mouseev.PassAs<ui::Event>())));
}

void EventConverterInProcess::KeyNotify(ui::EventType type,
                                        unsigned code,
                                        unsigned modifiers) {
  scoped_ptr<ui::KeyEvent> keyev(new ui::KeyEvent(type,
      KeyboardCodeFromNativeKeysym(code), modifiers, true));

  keyev.get()->set_character(CharacterCodeFromNativeKeySym(code, modifiers));

  PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::DispatchEventHelper, base::Passed(
          keyev.PassAs<ui::Event>())));
}

void EventConverterInProcess::CloseWidget(unsigned handle) {
  PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyCloseWidget, this, handle));
}

void EventConverterInProcess::OutputSizeChanged(unsigned width,
                                                unsigned height) {
  PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyOutputSizeChanged, this, width, height));
}

void EventConverterInProcess::WindowResized(unsigned handle,
                                            unsigned width,
                                            unsigned height) {
  PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyWindowResized, this, handle, width,
          height));
}

void EventConverterInProcess::SetWindowChangeObserver(
    WindowChangeObserver* observer) {
  observer_ = observer;
}

void EventConverterInProcess::SetOutputChangeObserver(
    OutputChangeObserver* observer) {
  output_observer_ = observer;
}

void EventConverterInProcess::NotifyPointerEnter(
    EventConverterInProcess* data, unsigned handle) {
  if (data->observer_)
    data->observer_->OnWindowEnter(handle);
}

void EventConverterInProcess::NotifyPointerLeave(
    EventConverterInProcess* data, unsigned handle) {
  if (data->observer_)
    data->observer_->OnWindowLeave(handle);
}

void EventConverterInProcess::NotifyButtonPress(
    EventConverterInProcess* data, unsigned handle) {
  if (data->observer_)
    data->observer_->OnWindowFocused(handle);
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

void EventConverterInProcess::DispatchEventHelper(
    scoped_ptr<ui::Event> key) {
  base::MessagePumpOzone::Current()->Dispatch(key.get());
}

}  // namespace ozonewayland
