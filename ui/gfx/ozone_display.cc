// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/gfx/ozone_display.h"

#include "base/logging.h"

namespace gfx {

// static
OzoneDisplay* OzoneDisplay::instance_ = NULL;

OzoneDisplay::OzoneDisplay() {
}

OzoneDisplay::~OzoneDisplay() {
}

OzoneDisplay* OzoneDisplay::GetInstance() {
  CHECK(instance_) << "No OzoneDisplay implementation set.";
  return instance_;
}

void OzoneDisplay::SetInstance(OzoneDisplay* instance) {
  CHECK(!instance_) << "Replacing set OzoneDisplay implementation.";
  instance_ = instance;
}

void OzoneDisplay::LookAheadOutputGeometry() {
}

}  // namespace gfx
