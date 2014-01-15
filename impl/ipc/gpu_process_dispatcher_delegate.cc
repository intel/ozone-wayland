// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/impl/ipc/gpu_process_dispatcher_delegate.h"

#include "base/bind.h"
#include "content/child/child_process.h"
#include "content/child/child_thread.h"
#include "ozone/impl/ipc/messages.h"

namespace {

content::ChildThread* GetProcessMainThread() {
  content::ChildProcess* process = content::ChildProcess::current();
  DCHECK(process);
  DCHECK(process->main_thread());
  return process ? process->main_thread() : NULL;
}

}

namespace ozonewayland {

GpuProcessDispatcherDelegate::GpuProcessDispatcherDelegate()
    : WaylandDispatcherDelegate() {
}

GpuProcessDispatcherDelegate::~GpuProcessDispatcherDelegate() {
}

void GpuProcessDispatcherDelegate::MotionNotify(float x, float y) {
  PostTaskOnMainLoop(base::Bind(&GpuProcessDispatcherDelegate::SendMotionNotify,
                                x,
                                y));
}

void GpuProcessDispatcherDelegate::ButtonNotify(unsigned handle,
                                                int state,
                                                int flags,
                                                float x,
                                                float y) {
  PostTaskOnMainLoop(base::Bind(&GpuProcessDispatcherDelegate::SendButtonNotify,
                                handle,
                                state,
                                flags,
                                x,
                                y));
}

void GpuProcessDispatcherDelegate::AxisNotify(float x,
                                              float y,
                                              int xoffset,
                                              int yoffset) {
  PostTaskOnMainLoop(base::Bind(&GpuProcessDispatcherDelegate::SendAxisNotify,
                                x,
                                y,
                                xoffset,
                                yoffset));
}

void GpuProcessDispatcherDelegate::PointerEnter(unsigned handle,
                                                float x,
                                                float y) {
  PostTaskOnMainLoop(base::Bind(&GpuProcessDispatcherDelegate::SendPointerEnter,
                                handle,
                                x,
                                y));
}

void GpuProcessDispatcherDelegate::PointerLeave(unsigned handle,
                                                float x,
                                                float y) {
  PostTaskOnMainLoop(base::Bind(&GpuProcessDispatcherDelegate::SendPointerLeave,
                                handle,
                                x,
                                y));
}

void GpuProcessDispatcherDelegate::KeyNotify(unsigned state,
                                             unsigned code,
                                             unsigned modifiers) {
  PostTaskOnMainLoop(base::Bind(&GpuProcessDispatcherDelegate::SendKeyNotify,
                                state,
                                code,
                                modifiers));
}

void GpuProcessDispatcherDelegate::OutputSizeChanged(unsigned width,
                                                     unsigned height) {
  PostTaskOnMainLoop(base::Bind(
      &GpuProcessDispatcherDelegate::SendOutputSizeChanged, width, height));
}

void GpuProcessDispatcherDelegate::SendMotionNotify(float x, float y) {
  content::ChildThread* thread = GetProcessMainThread();
  thread->Send(new WaylandInput_MotionNotify(x, y));
}

void GpuProcessDispatcherDelegate::SendButtonNotify(unsigned handle,
                                                    int state,
                                                    int flags,
                                                    float x,
                                                    float y) {
  content::ChildThread* thread = GetProcessMainThread();
  thread->Send(new WaylandInput_ButtonNotify(handle, state, flags, x, y));
}

void GpuProcessDispatcherDelegate::SendAxisNotify(float x,
                                                  float y,
                                                  int xoffset,
                                                  int yoffset) {
  content::ChildThread* thread = GetProcessMainThread();
  thread->Send(new WaylandInput_AxisNotify(x, y, xoffset, yoffset));
}

void GpuProcessDispatcherDelegate::SendPointerEnter(unsigned handle,
                                                    float x,
                                                    float y) {
  content::ChildThread* thread = GetProcessMainThread();
  thread->Send(new WaylandInput_PointerEnter(handle, x, y));
}

void GpuProcessDispatcherDelegate::SendPointerLeave(unsigned handle,
                                                    float x,
                                                    float y) {
  content::ChildThread* thread = GetProcessMainThread();
  thread->Send(new WaylandInput_PointerLeave(handle, x, y));
}

void GpuProcessDispatcherDelegate::SendKeyNotify(unsigned type,
                                                 unsigned code,
                                                 unsigned modifiers) {
  content::ChildThread* thread = GetProcessMainThread();
  thread->Send(new WaylandInput_KeyNotify(type, code, modifiers));
}

void GpuProcessDispatcherDelegate::SendOutputSizeChanged(unsigned width,
                                                         unsigned height) {
  content::ChildThread* thread = GetProcessMainThread();
  thread->Send(new WaylandInput_OutputSize(width, height));
}

}  // namespace ozonewayland
