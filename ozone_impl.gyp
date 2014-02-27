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
        'ui/ui.gypi',
        'impl/impl.gypi',
        'impl/ipc/ipc.gypi',
      ],
      'defines': [
        'OZONE_WAYLAND_IMPLEMENTATION',
      ],
      'sources': [
        'platform/ozone_export_wayland.h',
        'platform/ozone_platform_wayland.cc',
        'platform/ozone_platform_wayland.h',
      ],
      'conditions': [
        ['toolkit_views==1 and chromeos == 0', {
          'includes': [
            'impl/desktop_aura/impl_view.gypi',
          ],
          'dependencies': [
            '../ui/accessibility/accessibility.gyp:accessibility',
          ],
        }],
      ],
    },
  ]
}
