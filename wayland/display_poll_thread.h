#ifndef OZONE_WAYLAND_DISPATCHER_H_
#define OZONE_WAYLAND_DISPATCHER_H_

#include "base/threading/thread.h"

class wl_display;
namespace ozonewayland {
// This class lets you poll on a given Wayland display (passed in constructor),
// read any pending events coming from Wayland compositor and flush requests
// back. Caller should ensure that StopProcessingEvents is called before display
// is destroyed.
class WaylandDisplayPollThread : public base::Thread {
 public:
  explicit WaylandDisplayPollThread(wl_display* display);
  virtual ~WaylandDisplayPollThread();

  // Starts polling on wl_display fd and read/flush requests coming from Wayland
  // compositor.
  void StartProcessingEvents();
  // Stops polling and handling of any events from Wayland compositor.
  void StopProcessingEvents();
 private:
  static void DisplayRun(WaylandDisplayPollThread* data);
  bool active_ :1;
  int epoll_fd_;
  wl_display* display_;
  DISALLOW_COPY_AND_ASSIGN(WaylandDisplayPollThread);
};

}

#endif
