#ifndef OZONE_WAYLAND_DISPATCHER_H_
#define OZONE_WAYLAND_DISPATCHER_H_

#include "base/threading/thread.h"

namespace ozonewayland {
// WaylandDispatcher class is used by WaylandDisplay for reading pending events
// coming from Wayland compositor and flush requests back. WaylandDispatcher is
// performed entirely on a separate thread.
class WaylandDispatcher : public base::Thread {
 public:
  explicit WaylandDispatcher(int fd);
  virtual ~WaylandDispatcher();

  // Starts polling on the fd.
  void StartProcessingEvents();
  // Stops polling on the fd.
  void StopProcessingEvents();
 private:
  static void DisplayRun(WaylandDispatcher* data);
  bool active_ :1;
  int epoll_fd_;
  unsigned display_fd_;
  DISALLOW_COPY_AND_ASSIGN(WaylandDispatcher);
};

}

#endif
