// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/dispatcher.h"

#include "ozone/wayland/kbd_conversion.h"

#include "base/bind.h"
#include "base/message_loop/message_pump_ozone.h"
#include "content/child/child_thread.h"
#include "content/child/child_process.h"
#include "ui/base/events/event.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>

namespace ui {
WaylandDispatcher* WaylandDispatcher::instance_ = NULL;

// os-compatibility
extern "C" {
int osEpollCreateCloExec(void);

static int setCloExecOrClose(int fd)
{
  long flags;

  if (fd == -1)
    return -1;

  flags = fcntl(fd, F_GETFD);
  if (flags == -1)
    goto err;

  if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1)
    goto err;

  return fd;

err:
  close(fd);
  return -1;
}

int osEpollCreateCloExec(void)
{
  int fd;

#ifdef EPOLL_CLOEXEC
  fd = epoll_create1(EPOLL_CLOEXEC);
  if (fd >= 0)
    return fd;
  if (errno != EINVAL)
    return -1;
#endif

  fd = epoll_create(1);
  return setCloExecOrClose(fd);
}

} // os-compatibility


content::ChildThread* GetProcessMainThread()
{
  content::ChildProcess* process = content::ChildProcess::current();
  DCHECK(process);
  DCHECK(process->main_thread());
  return process ? process->main_thread() : NULL;
}

WaylandDispatcher::WaylandDispatcher(int fd)
    : Thread("WaylandDispatcher"),
      ignore_task_(false),
      running(false),
      epoll_fd_(0),
      display_fd_(fd)
{
  instance_ = this;
  if (display_fd_) {
    epoll_fd_ = osEpollCreateCloExec();
    struct epoll_event ep;
    ep.events = EPOLLIN | EPOLLOUT;
    ep.data.ptr = 0;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, display_fd_, &ep);
  }

  loop_ = base::MessageLoop::current();
  Options options;
  options.message_loop_type = base::MessageLoop::TYPE_IO;
  StartWithOptions(options);
  SetPriority(base::kThreadPriority_Background);
}

WaylandDispatcher::~WaylandDispatcher()
{
  ignore_task_ = true;
  loop_ = NULL;
  running = false;
  Stop();

  if (epoll_fd_) {
    close(epoll_fd_);
    epoll_fd_ = 0;
  }

  instance_ = NULL;
}

void WaylandDispatcher::PostTask(Task type)
{
  if (!IsRunning() || ignore_task_)
    return;

  switch (type) {
    case (Flush):
      message_loop_proxy()->PostTask(
                  FROM_HERE, base::Bind(&WaylandDispatcher::HandleFlush));
      break;
    case (Poll):
      if (epoll_fd_) {
        loop_ = base::MessageLoop::current();
        if (!running)
          message_loop_proxy()->PostTask(FROM_HERE, base::Bind(
                                          &WaylandDispatcher::DisplayRun, this));
      }
  default:
    break;
  }
}

void WaylandDispatcher::MessageLoopDestroyed()
{
  if (!IsRunning())
    return;

  ignore_task_ = true;
  loop_ = NULL;
  running = false;
  Stop();
}

void WaylandDispatcher::PostTaskOnMainLoop(
        const tracked_objects::Location& from_here, const base::Closure& task)
{
  if (ignore_task_ || !IsRunning() || !loop_)
    return;

  loop_->message_loop_proxy()->PostTask(from_here, task);
}

void WaylandDispatcher::DispatchEvent(scoped_ptr<ui::Event> event) {
  PostTaskOnMainLoop(FROM_HERE, base::Bind(&WaylandDispatcher::DispatchEventHelper,
                                           base::Passed(&event)));
}

void WaylandDispatcher::DispatchEventHelper(scoped_ptr<ui::Event> key) {
  base::MessagePumpOzone::Current()->Dispatch(key.get());
}

void WaylandDispatcher::HandleFlush()
{
  wl_display* waylandDisp = WaylandDisplay::GetInstance()->display();

  while (wl_display_prepare_read(waylandDisp) != 0)
    wl_display_dispatch_pending(waylandDisp);

  wl_display_flush(waylandDisp);
  wl_display_read_events(waylandDisp);
  wl_display_dispatch_pending(waylandDisp);
}

void  WaylandDispatcher::DisplayRun(WaylandDispatcher* data)
{
  struct epoll_event ep[16];
  int i, count, ret;

  data->running = 1;
  // Adopted from:
  // http://cgit.freedesktop.org/wayland/weston/tree/clients/window.c#n5531.
  while (1) {
    wl_display* waylandDisp = WaylandDisplay::GetInstance()->display();
    wl_display_dispatch_pending(waylandDisp);

    if (!data->running)
      break;

    ret = wl_display_flush(waylandDisp);
    if (ret < 0 && errno == EAGAIN) {
      ep[0].events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP;
      epoll_ctl(data->epoll_fd_, EPOLL_CTL_MOD, data->display_fd_, &ep[0]);
    } else if (ret < 0) {
      break;
    }

    count = epoll_wait(data->epoll_fd_, ep, 16, -1);
    for (i = 0; i < count; i++) {
      struct epoll_event eps;
      int ret;
      uint32_t event = ep[i].events;

      if (event & EPOLLERR || event & EPOLLHUP)
        return;

      if (event & EPOLLIN) {
        ret = wl_display_dispatch(waylandDisp);
        if (ret == -1)
          return;
      }

      if (event & EPOLLOUT) {
        ret = wl_display_flush(waylandDisp);
        if (ret == 0) {
          eps.events = EPOLLIN | EPOLLERR | EPOLLHUP;
          epoll_ctl(data->epoll_fd_, EPOLL_CTL_MOD, data->display_fd_, &eps);
        } else if (ret == -1 && errno != EAGAIN) {
          return;
        }
      }
    }
  }
}

void WaylandDispatcher::SendMotionNotify(float x, float y)
{
  content::ChildThread* thread = GetProcessMainThread();
  thread->Send(new WaylandInput_MotionNotify(x, y));
}

void WaylandDispatcher::SendButtonNotify(int state, int flags, float x, float y)
{
  content::ChildThread* thread = GetProcessMainThread();
  thread->Send(new WaylandInput_ButtonNotify(state, flags, x, y));
}

void WaylandDispatcher::SendAxisNotify(float x, float y, float xoffset,
                                       float yoffset)
{
  content::ChildThread* thread = GetProcessMainThread();
  thread->Send(new WaylandInput_AxisNotify(x, y, xoffset, yoffset));
}

void WaylandDispatcher::SendPointerEnter(float x, float y)
{
  content::ChildThread* thread = GetProcessMainThread();
  thread->Send(new WaylandInput_PointerEnter(x, y));
}

void WaylandDispatcher::SendPointerLeave(float x, float y)
{
  content::ChildThread* thread = GetProcessMainThread();
  thread->Send(new WaylandInput_PointerLeave(x, y));
}

void WaylandDispatcher::SendKeyNotify(unsigned type, unsigned code,
                                      unsigned modifiers)
{
  content::ChildThread* thread = GetProcessMainThread();
  thread->Send(new WaylandInput_KeyNotify(type, code, modifiers));
}

void WaylandDispatcher::SendOutputSizeChanged(unsigned width, unsigned height)
{
  content::ChildThread* thread = GetProcessMainThread();
  thread->Send(new WaylandInput_OutputSize(width, height));
}

void WaylandDispatcher::MotionNotify(float x, float y)
{
  if (epoll_fd_) {
    if (!running)
      return;
    PostTaskOnMainLoop(FROM_HERE, base::Bind(
                        &WaylandDispatcher::SendMotionNotify, x, y));
  } else {
    scoped_ptr<MouseEvent> mouseev(new MouseEvent(ui::ET_MOUSE_MOVED,
                                                  gfx::Point(x, y),
                                                  gfx::Point(x, y), 0));

    PostTaskOnMainLoop(FROM_HERE, base::Bind(
                        &WaylandDispatcher::DispatchEventHelper, base::Passed(
                                              mouseev.PassAs<ui::Event>())));
  }
}

void WaylandDispatcher::ButtonNotify(int state, int flags, float x, float y)
{
  if (epoll_fd_) {
    if (!running)
      return;
    PostTaskOnMainLoop(FROM_HERE, base::Bind(&WaylandDispatcher::SendButtonNotify,
                                             state, flags, x, y));
  } else {
    EventType type;
    if (state == 1)
      type = ui::ET_MOUSE_PRESSED;
    else
      type = ui::ET_MOUSE_RELEASED;

    scoped_ptr<MouseEvent> mouseev(new MouseEvent(type, gfx::Point(x, y),
                                                  gfx::Point(x, y), flags));

    PostTaskOnMainLoop(FROM_HERE, base::Bind(
                        &WaylandDispatcher::DispatchEventHelper, base::Passed(
                                              mouseev.PassAs<ui::Event>())));
  }
}

void WaylandDispatcher::AxisNotify(float x, float y, float xoffset, float yoffset)
{
  if (epoll_fd_) {
    if (!running)
      return;

    PostTaskOnMainLoop(FROM_HERE, base::Bind(&WaylandDispatcher::SendAxisNotify,
                                             x, y, xoffset, yoffset));
  } else {
    MouseEvent mouseev(ui::ET_MOUSEWHEEL, gfx::Point(x,y), gfx::Point(x,y), 0);
    scoped_ptr<MouseWheelEvent> wheelev(new MouseWheelEvent(mouseev, xoffset,
                                                            yoffset));
    PostTaskOnMainLoop(FROM_HERE, base::Bind(
                        &WaylandDispatcher::DispatchEventHelper, base::Passed(
                                              wheelev.PassAs<ui::Event>())));
  }
}

void WaylandDispatcher::PointerEnter(float x, float y)
{
  if (epoll_fd_) {
    if (!running)
      return;
    PostTaskOnMainLoop(FROM_HERE, base::Bind(&WaylandDispatcher::SendPointerEnter,
                                             x, y));
  } else {
    scoped_ptr<MouseEvent> mouseev(new MouseEvent(ui::ET_MOUSE_ENTERED,
                                                  gfx::Point(x, y),
                                                  gfx::Point(x, y), 0));

    PostTaskOnMainLoop(FROM_HERE, base::Bind(
                        &WaylandDispatcher::DispatchEventHelper, base::Passed(
                                              mouseev.PassAs<ui::Event>())));
  }
}

void WaylandDispatcher::PointerLeave(float x, float y)
{
  if (epoll_fd_) {
    if (!running)
      return;
    PostTaskOnMainLoop(FROM_HERE, base::Bind(&WaylandDispatcher::SendPointerLeave,
                                             x, y));
  } else {
    scoped_ptr<MouseEvent> mouseev(new MouseEvent(ui::ET_MOUSE_EXITED,
                                                  gfx::Point(x, y),
                                                  gfx::Point(x, y),0));
    PostTaskOnMainLoop(FROM_HERE, base::Bind(
                        &WaylandDispatcher::DispatchEventHelper, base::Passed(
                                              mouseev.PassAs<ui::Event>())));
  }
}

void WaylandDispatcher::KeyNotify(unsigned state, unsigned code,
                                  unsigned modifiers)
{
  if (epoll_fd_) {
    if (!running)
      return;
    PostTaskOnMainLoop(FROM_HERE, base::Bind(&WaylandDispatcher::SendKeyNotify,
                                             state, code, modifiers));
  } else {
    EventType type;
    if (state)
      type = ET_KEY_PRESSED;
    else
      type = ET_KEY_RELEASED;

    scoped_ptr<KeyEvent> keyev(new KeyEvent(type, ui::KeyboardCodeFromXKeysym(code),
                                            modifiers, true));
    PostTaskOnMainLoop(FROM_HERE, base::Bind(
                        &WaylandDispatcher::DispatchEventHelper, base::Passed(
                                              keyev.PassAs<ui::Event>())));
  }
}

void WaylandDispatcher::OutputSizeChanged(unsigned width, unsigned height)
{
  if (!running || !epoll_fd_)
    return;

  PostTaskOnMainLoop(FROM_HERE, base::Bind(
                      &WaylandDispatcher::SendOutputSizeChanged, width, height));
}

}  // namespace ui
