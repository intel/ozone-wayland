# Copyright 2013 The Chromium Authors. All rights reserved.
# Copyright 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'wayland',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/skia/skia.gyp:skia',
        'wayland/wayland.gyp:wayland_toolkit'
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        'impl/desktop_factory_wayland.cc',
        'impl/desktop_factory_wayland.h',
        'impl/desktop_root_window_host_wayland.cc',
        'impl/desktop_root_window_host_wayland.h',
        'impl/desktop_drag_drop_client_wayland.cc',
        'impl/desktop_drag_drop_client_wayland.h',
        'impl/desktop_screen_wayland.cc',
        'impl/desktop_screen_wayland.h',
        'impl/event_factory_wayland.cc',
        'impl/event_factory_wayland.h',
        'impl/ozone_display.cc',
        'impl/ozone_display.h',
        'impl/ozone_platform_wayland.cc',
        'impl/ozone_platform_wayland.h',
        'impl/ipc/child_process_observer.h',
        'impl/ipc/child_process_observer.cc',
        'impl/ipc/display_channel.h',
        'impl/ipc/display_channel.cc',
        'impl/ipc/display_channel_host.h',
        'impl/ipc/display_channel_host.cc',
        'impl/ipc/messages.h',
        'impl/ipc/message_generator.h',
        'impl/ipc/message_generator.cc',
      ],
    },
  ]
}
