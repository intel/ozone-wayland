# Copyright 2013 The Chromium Authors. All rights reserved.
# Copyright 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'wayland',
      'type': '<(component)',
      'dependencies': [
        '<(DEPTH)/skia/skia.gyp:skia',
        '<(DEPTH)/base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        'wayland/wayland.gyp:wayland_toolkit'
      ],
      'include_dirs': [
        '..',
      ],
      'includes': [
        'ui/events/events.gypi',
      ],
      'defines': [
        'OZONE_WAYLAND_IMPLEMENTATION',
      ],
      'sources': [
        'impl/desktop_factory_wayland.cc',
        'impl/desktop_factory_wayland.h',
        'impl/desktop_window_tree_host_wayland.cc',
        'impl/desktop_window_tree_host_wayland.h',
        'impl/desktop_drag_drop_client_wayland.cc',
        'impl/desktop_drag_drop_client_wayland.h',
        'impl/desktop_screen_wayland.cc',
        'impl/desktop_screen_wayland.h',
        'impl/event_factory_wayland.cc',
        'impl/event_factory_wayland.h',
        'impl/fullscreen_wayland.cc',
        'impl/ozone_display.cc',
        'impl/ozone_display.h',
        'impl/window_tree_host_delegate_wayland.cc',
        'impl/window_tree_host_delegate_wayland.h',
        'impl/ipc/child_process_observer.h',
        'impl/ipc/child_process_observer.cc',
        'impl/ipc/display_channel.h',
        'impl/ipc/display_channel.cc',
        'impl/ipc/display_channel_host.h',
        'impl/ipc/display_channel_host.cc',
        'impl/ipc/messages.h',
        'impl/ipc/message_generator.h',
        'impl/ipc/message_generator.cc',
        'platform/ozone_export_wayland.h',
        'platform/ozone_platform_wayland.cc',
        'platform/ozone_platform_wayland.h',
      ],
    },
  ]
}
