// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_DELEGATE_H_
#define OZONE_WAYLAND_DELEGATE_H_

#include "ozone/wayland_display.h"

namespace base {
namespace wayland {
union WaylandEvent;
}
}

namespace ui {

// WaylandDelegate is an interface for processing Wayland events.
class WaylandDelegate {
 public:
  virtual ~WaylandDelegate() {}

  virtual void OnMouseEvent(base::wayland::WaylandEvent* event) = 0;
  virtual void OnKeyNotify(base::wayland::WaylandEvent* event) = 0;
  virtual void OnKeyboardEnter(base::wayland::WaylandEvent* event) = 0;
  virtual void OnKeyboardLeave(base::wayland::WaylandEvent* event) = 0;
  virtual void OnMouseEnter(base::wayland::WaylandEvent* event) = 0;
  virtual void OnMouseLeave(base::wayland::WaylandEvent* event) = 0;
  virtual void OnGeometryChange(base::wayland::WaylandEvent* event) = 0;
	virtual void OnBoundsChanged(const gfx::Rect& old_bounds,
    const gfx::Rect& new_bounds) = 0;
  virtual void OnWindowDestroying() = 0;
  virtual void OnWindowDestroyed() = 0;

	// Returns the non-client component (see hit_test.h) containing |point|, in
  // window coordinates.
  virtual int GetNonClientComponent(const gfx::Point& point) const = 0;
};

}  // namespace ui

#endif  // OZONE_WAYLAND_DELEGATE_H_
