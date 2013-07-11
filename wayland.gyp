# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'wayland',
      'type': 'static_library',
      'dependencies': [
        '../../build/linux/system.gyp:wayland',
        '../../base/base.gyp:base',
        '../../base/base.gyp:base_i18n',
        '../../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '../../skia/skia.gyp:skia',
        '../compositor/compositor.gyp:compositor',
        '../ui.gyp:ui',
        '../ui.gyp:ui_resources',
      ],
      'include_dirs': [
        '.',
        '../..',
      ],
      'sources': [
        'wayland_display.cc',
        'wayland_display.h',
        'wayland_task.cc',
        'wayland_task.h',
        'wayland_input_device.cc',
        'wayland_input_device.h',
        'wayland_input_method_event_filter.cc',
        'wayland_input_method_event_filter.h',
        'wayland_screen.cc',
        'wayland_screen.h',
        'wayland_delegate.h',
        'wayland_window.cc',
        'wayland_window.h',
      ],
    },
  ],
}
