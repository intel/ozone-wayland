// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/input/cursor.h"

#include <sys/mman.h>
#include <vector>

#include "base/logging.h"
#include "base/memory/shared_memory.h"
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
    shm_(shm),
    current_cursor_(CURSOR_UNSET),
    buffer_(NULL),
    sh_memory_(new base::SharedMemory()),
    width_(0),
    height_(0) {
  DCHECK(WaylandCursorData::GetInstance());
  WaylandDisplay* display = WaylandDisplay::GetInstance();
  pointer_surface_ = wl_compositor_create_surface(display->GetCompositor());
}

WaylandCursor::~WaylandCursor() {
  DCHECK(pointer_surface_);
  wl_surface_destroy(pointer_surface_);
  if (buffer_)
    wl_buffer_destroy(buffer_);

  if (sh_memory_->handle().fd) {
    sh_memory_->Unmap();
    sh_memory_->Close();
  }
  delete sh_memory_;
}

void WaylandCursor::Clear() {
  WaylandCursorData::DestroyCursorData();
}

void WaylandCursor::InitializeCursorData(wl_shm* shm) {
  WaylandCursorData::InitializeCursorData(shm);
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

void WaylandCursor::UpdateBitmap(const std::vector<SkBitmap>& cursor_image,
                                 const gfx::Point& location,
                                 uint32_t serial) {
  if (!input_pointer_)
    return;

  if (!cursor_image.size()) {
     HideCursor(serial);
     return;
  }

  const SkBitmap& image = cursor_image[0];
  int width = image.width();
  int height = image.height();
  if (!width || !height) {
     HideCursor(serial);
     return;
  }

  if (!CreateSHMBuffer(width, height)) {
    LOG(INFO) << "Failed to create SHM buffer for Cursor Bitmap.";
    return;
  }

  // The |bitmap| contains ARGB image, so just copy it.
  memcpy(sh_memory_->memory(), image.getPixels(), width_ * height_ * 4);

  wl_pointer_set_cursor(input_pointer_,
                        serial,
                        pointer_surface_,
                        location.x(),
                        location.y());

  if (buffer_) {
    struct wl_surface* surface = pointer_surface_;
    wl_surface_attach(surface, buffer_, 0, 0);
    wl_surface_damage(surface, 0, 0, width_, height_);
    wl_surface_commit(surface);
  }
}

bool WaylandCursor::CreateSHMBuffer(int width, int height) {
  if (width == width_ && height == height_)
    return true;

  struct wl_shm_pool *pool;
  int size, stride;

  width_ = width;
  height_ = height;
  stride = width_ * 4;
  SkImageInfo info = SkImageInfo::MakeN32Premul(width_, height_);
  size = info.getSafeSize(stride);

  if (sh_memory_->handle().fd) {
    sh_memory_->Unmap();
    sh_memory_->Close();
  }

  if (!sh_memory_->CreateAndMapAnonymous(size)) {
    LOG(INFO) << "Create and mmap failed.";
    return false;
  }

  pool = wl_shm_create_pool(shm_,
                            sh_memory_->handle().fd,
                            size);
  buffer_ = wl_shm_pool_create_buffer(pool, 0,
                                      width_, height_,
                                      stride, WL_SHM_FORMAT_ARGB8888);
  wl_shm_pool_destroy(pool);
  return true;
}

void WaylandCursor::HideCursor(uint32_t serial) {
  width_ = 0;
  height_ = 0;

  if (buffer_) {
    wl_buffer_destroy(buffer_);
    buffer_ = NULL;
  }

  if (sh_memory_->handle().fd) {
    sh_memory_->Unmap();
    sh_memory_->Close();
  }

  wl_pointer_set_cursor(input_pointer_,
                        serial,
                        NULL,
                        0,
                        0);
}

void WaylandCursor::SetInputPointer(wl_pointer* pointer) {
  if (input_pointer_ == pointer)
    return;

  if (input_pointer_)
    wl_pointer_destroy(input_pointer_);

  input_pointer_ = pointer;
}

}  // namespace ozonewayland
