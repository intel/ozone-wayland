// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland_cursor.h"
#include <stdio.h>
namespace ui {

WaylandCursor::WaylandCursor(wl_compositor* compositor, wl_shm* shm) :
    cursor_theme_(NULL),
    cursors_(NULL),
    input_pointer_(NULL),
    pointer_surface_(NULL),
    buffer_(NULL),
    cursor_type_(CURSOR_UNSET)
{
  pointer_surface_ = wl_compositor_create_surface(compositor);
  initialize(shm);
}

WaylandCursor::~WaylandCursor()
{
  Destroy();
}

void WaylandCursor::initialize(wl_shm* shm)
{
  const char *cursor_names[] = {
    "bottom_left_corner",
    "bottom_right_corner",
    "bottom_side",
    "grabbing",
    "left_ptr",
    "left_side",
    "right_side",
    "top_left_corner",
    "top_right_corner",
    "top_side",
    "xterm",
    "hand1",
  };

  // (kalyan) We should be able to configure the size of cursor and theme name.
  unsigned int i, array_size = (sizeof(cursor_names) / sizeof(cursor_names[0])) -1;
  cursor_theme_ = wl_cursor_theme_load(NULL, 24, shm);
  cursors_ = new wl_cursor*[array_size];
  memset(cursors_, 0, sizeof(wl_cursor*) * array_size);

  for(i = 0; i < array_size; i++)
    cursors_[i] = wl_cursor_theme_get_cursor(cursor_theme_, cursor_names[i]);
}

void WaylandCursor::Destroy()
{
  if(cursor_theme_)
  {
    wl_cursor_theme_destroy(cursor_theme_);
    cursor_theme_ = NULL;
  }

  if(cursors_)
  {
    delete[] cursors_;
    cursors_ = NULL;
  }

  input_pointer_ = NULL;
}

void WaylandCursor::SetPointer(wl_pointer* pointer)
{
  input_pointer_ = pointer;
  cursor_type_ = CURSOR_UNSET;
}

void WaylandCursor::Update(CursorType type, uint32_t serial)
{
  if (!input_pointer_)
    return;

  cursor_type_ = type;
  struct wl_cursor *cursor = cursors_[cursor_type_ - 1];
  if (!cursor)
    return;

  struct wl_cursor_image *image = cursor->images[0];
  buffer_ = wl_cursor_image_get_buffer(image);
  wl_pointer_set_cursor(input_pointer_, serial, pointer_surface_, image->hotspot_x, image->hotspot_y);
  wl_surface_attach(pointer_surface_, buffer_, 0, 0);
  wl_surface_damage(pointer_surface_, 0, 0, image->width, image->height);
  wl_surface_commit(pointer_surface_);
}

}  // namespace ui

