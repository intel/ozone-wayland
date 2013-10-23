# Copyright 2013 The Chromium Authors. All rights reserved.
# Copyright 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables':  {
    'ozone_platform_wayland%': 1,
  },
  'conditions': [
    ['<(ozone_platform_wayland) == 1', {
      'variables':  {
        'external_ozone_platform_deps': [
          '<(DEPTH)/ozone/ozone_impl.gyp:wayland',
        ],
        'external_ozone_platforms': [
          'wayland'
        ],
        'ozone_platform%': 'wayland',
      },
    }],
  ],
}
