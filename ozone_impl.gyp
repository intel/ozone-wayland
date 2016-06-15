# Copyright 2013 The Chromium Authors. All rights reserved.
# Copyright 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
   'includes': [
       'ui/ui.gypi',
    ],
  'targets': [
    {
      'target_name': 'wayland',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/skia/skia.gyp:skia',
        '<(DEPTH)/base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '<(DEPTH)/ui/events/ozone/events_ozone.gyp:events_ozone_evdev',
        '<(DEPTH)/ui/gfx/ipc/gfx_ipc.gyp:gfx_ipc',
        'wayland/wayland.gyp:wayland_toolkit'
      ],
      'include_dirs': [
        '..',
      ],
        'defines': [
        'OZONE_WAYLAND_IMPLEMENTATION',
      ],
      'sources': [
        'media/media_ozone_platform_wayland.cc',
        'media/media_ozone_platform_wayland.h',
	'platform/client_native_pixmap_factory_wayland.cc',
	'platform/client_native_pixmap_factory_wayland.h',
        'platform/desktop_platform_screen.h',
	'platform/desktop_platform_screen_delegate.h',
        'platform/ozone_export_wayland.h',
	'platform/messages.h',
	'platform/message_generator.h',
	'platform/message_generator.cc',
	'platform/ozone_gpu_platform_support_host.h',
	'platform/ozone_gpu_platform_support_host.cc',
        'platform/ozone_platform_wayland.cc',
        'platform/ozone_platform_wayland.h',
        'platform/ozone_wayland_window.cc',
        'platform/ozone_wayland_window.h',
	'platform/window_constants.h',
        'platform/window_manager_wayland.cc',
        'platform/window_manager_wayland.h',
      ],
    },
  ]
}
