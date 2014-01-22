// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/events/remote_event_dispatcher.h"

#include "base/bind.h"
#include "content/child/child_process.h"
#include "content/child/child_thread.h"
#include "ozone/impl/ipc/messages.h"

namespace {

content::ChildThread* GetProcessMainThread() {
  content::ChildProcess* process = content::ChildProcess::current();
  DCHECK(process && process->main_thread());
  return process ? process->main_thread() : NULL;
}

}

namespace ozonewayland {

RemoteEventDispatcher::RemoteEventDispatcher()
    : EventConverterOzoneWayland() {
}

RemoteEventDispatcher::~RemoteEventDispatcher() {
}

void RemoteEventDispatcher::MotionNotify(float x, float y) {
  PostTaskOnMainLoop(base::Bind(&RemoteEventDispatcher::SendMotionNotify,
                                x,
                                y));
}

void RemoteEventDispatcher::ButtonNotify(unsigned handle,
                                         ui::EventType type,
                                         ui::EventFlags flags,
                                         float x,
                                         float y) {
  PostTaskOnMainLoop(base::Bind(&RemoteEventDispatcher::SendButtonNotify,
                                handle,
                                type,
                                flags,
                                x,
                                y));
}

void RemoteEventDispatcher::AxisNotify(float x,
                                       float y,
                                       int xoffset,
                                       int yoffset) {
  PostTaskOnMainLoop(base::Bind(&RemoteEventDispatcher::SendAxisNotify,
                                x,
                                y,
                                xoffset,
                                yoffset));
}

void RemoteEventDispatcher::PointerEnter(unsigned handle,
                                         float x,
                                         float y) {
  PostTaskOnMainLoop(base::Bind(&RemoteEventDispatcher::SendPointerEnter,
                                handle,
                                x,
                                y));
}

void RemoteEventDispatcher::PointerLeave(unsigned handle,
                                         float x,
                                         float y) {
  PostTaskOnMainLoop(base::Bind(&RemoteEventDispatcher::SendPointerLeave,
                                handle,
                                x,
                                y));
}

void RemoteEventDispatcher::KeyNotify(ui::EventType state,
                                      unsigned code,
                                      unsigned modifiers) {
  PostTaskOnMainLoop(base::Bind(&RemoteEventDispatcher::SendKeyNotify,
                                state,
                                code,
                                modifiers));
}

void RemoteEventDispatcher::OutputSizeChanged(unsigned width,
                                              unsigned height) {
  PostTaskOnMainLoop(base::Bind(
      &RemoteEventDispatcher::SendOutputSizeChanged, width, height));
}

void RemoteEventDispatcher::CloseWidget(unsigned handle) {
  PostTaskOnMainLoop(base::Bind(
      &RemoteEventDispatcher::SendCloseWidget, handle));
}

void RemoteEventDispatcher::SendMotionNotify(float x, float y) {
  Send(new WaylandInput_MotionNotify(x, y));
}

void RemoteEventDispatcher::SendButtonNotify(unsigned handle,
                                             ui::EventType type,
                                             ui::EventFlags flags,
                                             float x,
                                             float y) {
  Send(new WaylandInput_ButtonNotify(handle, type, flags, x, y));
}

void RemoteEventDispatcher::SendAxisNotify(float x,
                                           float y,
                                           int xoffset,
                                           int yoffset) {
  Send(new WaylandInput_AxisNotify(x, y, xoffset, yoffset));
}

void RemoteEventDispatcher::SendPointerEnter(unsigned handle,
                                             float x,
                                             float y) {
  Send(new WaylandInput_PointerEnter(handle, x, y));
}

void RemoteEventDispatcher::SendPointerLeave(unsigned handle,
                                             float x,
                                             float y) {
  Send(new WaylandInput_PointerLeave(handle, x, y));
}

void RemoteEventDispatcher::SendKeyNotify(ui::EventType type,
                                          unsigned code,
                                          unsigned modifiers) {
  Send(new WaylandInput_KeyNotify(type, code, modifiers));
}

void RemoteEventDispatcher::SendOutputSizeChanged(unsigned width,
                                                  unsigned height) {
  Send(new WaylandInput_OutputSize(width, height));
}

void RemoteEventDispatcher::SendCloseWidget(unsigned handle) {
  Send(new WaylandInput_CloseWidget(handle));
}

void RemoteEventDispatcher::Send(IPC::Message* message) {
  content::ChildThread* thread = GetProcessMainThread();
  thread->Send(message);
}

}  // namespace ozonewayland
