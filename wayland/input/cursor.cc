// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/input/cursor.h"

#include <vector>

#include "base/logging.h"
#include "ozone/wayland/display.h"

namespace ozonewayland {
// This number should be equal to size of array defined in WaylandCursorData
// constructor.
const unsigned TotalCursorTypes = 24;

class WaylandCursorData {
 public:
  explicit WaylandCursorData(wl_shm* shm);
  ~WaylandCursorData();

  static WaylandCursorData* GetInstance() {
    return impl_;
  }

  static void InitializeCursorData(wl_shm* shm) {
    if (!impl_)
      impl_ = new WaylandCursorData(shm);
  }

  static void DestroyCursorData() {
    if (impl_) {
      delete impl_;
      impl_ = NULL;
    }
  }

  struct wl_cursor_image* GetCursorImage(WaylandCursor::CursorType index);

 private:
  wl_cursor_theme* cursor_theme_;
  // All supported Cursor types.
  std::vector<wl_cursor*> cursors_;
  static WaylandCursorData* impl_;
  DISALLOW_COPY_AND_ASSIGN(WaylandCursorData);
};

WaylandCursorData* WaylandCursorData::impl_ = NULL;

WaylandCursorData::WaylandCursorData(wl_shm* shm)
    : cursor_theme_(NULL),
      cursors_(std::vector<wl_cursor*>(TotalCursorTypes)) {
  // This list should be always in sync with WaylandCursor::CursorType
  const char* cursor_names[] = {
    "default",
    "bottom_left_corner",
    "bottom_right_corner",
    "bottom_side",
    "cross",
    "fleur",
    "grabbing",
    "left_ptr",
    "left_side",
    "left_arrow",
    "right_side",
    "top_left_corner",
    "top_left_arrow",
    "top_right_corner",
    "top_side",
    "up_arrow",
    "wait",
    "watch",
    "xterm",
    "hand1",
    "text",
    "question_arrow",
    "v_double_arrow",
    "h_double_arrow"
  };

  // (kalyan) We should be able to configure the size of cursor and theme name.
  cursor_theme_ = wl_cursor_theme_load(NULL, 24, shm);
  DCHECK(cursor_theme_);

  for (unsigned i = 0; i < TotalCursorTypes; i++)
    cursors_[i] = wl_cursor_theme_get_cursor(cursor_theme_, cursor_names[i]);
}

struct wl_cursor_image* WaylandCursorData::GetCursorImage(
    WaylandCursor::CursorType type) {
  int index = type - 1;
  const struct wl_cursor* cursor = cursors_.at(index);
  if (!cursor)
    return NULL;

  return cursor->images[0];
}

WaylandCursorData::~WaylandCursorData() {
  wl_cursor_theme_destroy(cursor_theme_);

  if (!cursors_.empty())
    cursors_.clear();
}

WaylandCursor::WaylandCursor(wl_shm* shm) : input_pointer_(NULL),
    pointer_surface_(NULL),
    current_cursor_(CURSOR_UNSET) {
  WaylandCursorData::InitializeCursorData(shm);
  WaylandDisplay* display = WaylandDisplay::GetInstance();
  pointer_surface_ = wl_compositor_create_surface(display->GetCompositor());
}

WaylandCursor::~WaylandCursor() {
  DCHECK(pointer_surface_);
  wl_surface_destroy(pointer_surface_);
}

void WaylandCursor::Clear() {
  WaylandCursorData::DestroyCursorData();
}

void WaylandCursor::Update(CursorType type, uint32_t serial) {
  if (!input_pointer_)
    return;

  DCHECK(type != CURSOR_UNSET);
  CursorType cursor_type = type;
  wl_cursor_image* image = WaylandCursorData::GetInstance()->GetCursorImage(
      cursor_type);

  if (!image) {
    LOG(INFO) << "The current cursor theme does not have a cursor for type "
              << cursor_type << ". Falling back to the default cursor.";
    // The cursor currently being displayed is already the default one, so we
    // can just continue showing it.
    if (current_cursor_ == CURSOR_LEFT_PTR)
      return;

    cursor_type = CURSOR_LEFT_PTR;
    image = WaylandCursorData::GetInstance()->GetCursorImage(cursor_type);
    DCHECK(image);
  }

  current_cursor_ = cursor_type;
  struct wl_buffer* buffer = wl_cursor_image_get_buffer(image);
  int width = image->width;
  int height = image->height;
  wl_pointer_set_cursor(input_pointer_,
                        serial,
                        pointer_surface_,
                        image->hotspot_x,
                        image->hotspot_y);

  struct wl_surface* surface = pointer_surface_;
  wl_surface_attach(surface, buffer, 0, 0);
  wl_surface_damage(surface, 0, 0, width, height);
  wl_surface_commit(surface);
}

void WaylandCursor::SetInputPointer(wl_pointer* pointer) {
  if (input_pointer_ == pointer)
    return;

  if (input_pointer_)
    wl_pointer_destroy(input_pointer_);

  input_pointer_ = pointer;
}

}  // namespace ozonewayland
