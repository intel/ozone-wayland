// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Multiply-included message file, hence no include guard here.

#include <string>

#include "base/basictypes.h"
#include "base/memory/shared_memory.h"
#include "base/strings/string16.h"
#include "ipc/ipc_message_macros.h"
#include "ipc/ipc_message_utils.h"
#include "ipc/ipc_param_traits.h"
#include "ipc/param_traits_macros.h"
#include "ozone/ui/events/window_constants.h"
#include "ui/events/event_constants.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/ipc/gfx_param_traits.h"


#define IPC_MESSAGE_START LastIPCMsgStart

IPC_ENUM_TRAITS_MAX_VALUE(ui::EventFlags,
                          ui::EF_ALTGR_DOWN)
IPC_ENUM_TRAITS_MAX_VALUE(ui::EventType,
                          ui::ET_LAST)
IPC_ENUM_TRAITS_MAX_VALUE(ui::WidgetState,
                          ui::DESTROYED)
IPC_ENUM_TRAITS_MAX_VALUE(ui::WidgetType,
                          ui::POPUP)

IPC_MESSAGE_CONTROL2(WaylandInput_InitializeXKB,  // NOLINT(readability/fn_size)
                     base::SharedMemoryHandle /*fd*/,
                     uint32_t /*size*/)

IPC_MESSAGE_CONTROL3(WaylandInput_KeyNotify,  // NOLINT(readability/fn_size)
                     ui::EventType /*type*/,
                     unsigned /*code*/,
                     int /*device_id*/)

IPC_MESSAGE_CONTROL3(  // NOLINT(readability/fn_size)
    WaylandInput_VirtualKeyNotify,
    ui::EventType /*type*/,
    uint32_t /*key*/,
    int /*device_id*/)

IPC_MESSAGE_CONTROL2(WaylandInput_MotionNotify,  // NOLINT(readability/fn_size)
                     float /*x*/,
                     float /*y*/)

IPC_MESSAGE_CONTROL5(WaylandInput_ButtonNotify,  // NOLINT(readability/fn_size)
                     unsigned /*handle*/,
                     ui::EventType /*type*/,
                     ui::EventFlags /*flags*/,
                     float /*x*/,
                     float /*y*/)

IPC_MESSAGE_CONTROL5(WaylandInput_TouchNotify,  // NOLINT(readability/fn_size)
                     ui::EventType /*type*/,
                     float /*x*/,
                     float /*y*/,
                     int32_t /*touch_id*/,
                     uint32_t /*time_stamp*/)

IPC_MESSAGE_CONTROL4(WaylandInput_AxisNotify,  // NOLINT(readability/fn_size)
                     float /*x*/,
                     float /*y*/,
                     int /*x_offset*/,
                     int /*y_offset*/)

IPC_MESSAGE_CONTROL3(WaylandInput_PointerEnter,  // NOLINT(readability/fn_size)
                     unsigned /*handle*/,
                     float /*x*/,
                     float /*y*/)

IPC_MESSAGE_CONTROL3(WaylandInput_PointerLeave,  // NOLINT(readability/fn_size)
                     unsigned /*handle*/,
                     float /*x*/,
                     float /*y*/)

IPC_MESSAGE_CONTROL2(WaylandInput_OutputSize,  // NOLINT(readability/fn_size)
                     unsigned /*width*/,
                     unsigned /*height*/)

IPC_MESSAGE_CONTROL1(WaylandInput_CloseWidget,  // NOLINT(readability/fn_size)
                     unsigned /*handle*/)

IPC_MESSAGE_CONTROL3(WaylandWindow_Resized,  // NOLINT(readability/fn_size)
                     unsigned /* window handle */,
                     unsigned /* width */,
                     unsigned /* height */)

IPC_MESSAGE_CONTROL1(WaylandWindow_Unminimized,  // NOLINT(readability/fn_size)
                     unsigned /*handle*/)

IPC_MESSAGE_CONTROL1(WaylandWindow_DeActivated,  // NOLINT(readability/fn_size)
                     unsigned /*handle*/)

IPC_MESSAGE_CONTROL1(WaylandWindow_Activated,  // NOLINT(readability/fn_size)
                     unsigned /*handle*/)

IPC_MESSAGE_CONTROL2(WaylandWindow_State,  // NOLINT(readability/fn_size)
                     unsigned /* window handle */,
                     ui::WidgetState /*state*/)

IPC_MESSAGE_CONTROL5(WaylandWindow_Create,  // NOLINT(readability/fn_size)
                     unsigned /* window handle */,
                     unsigned /* window parent */,
                     unsigned /* x */,
                     unsigned /* y */,
                     ui::WidgetType /* window type */)

IPC_MESSAGE_CONTROL2(WaylandWindow_Title,  // NOLINT(readability/fn_size)
                     unsigned /* window handle */,
                     base::string16 /* window title */)

IPC_MESSAGE_CONTROL1(WaylandWindow_Cursor,  // NOLINT(readability/fn_size)
                     int /* cursor type */)

IPC_MESSAGE_CONTROL0(WaylandWindow_ImeReset)  // NOLINT(readability/fn_size)

IPC_MESSAGE_CONTROL1(WaylandWindow_ImeCaretBoundsChanged, // NOLINT(readability/
                     gfx::Rect /* ImeCaretBoundsChanged */)  //      fn_size)

IPC_MESSAGE_CONTROL0(WaylandWindow_ShowInputPanel)  // NOLINT(readability/
                                                   //         fn_size)

IPC_MESSAGE_CONTROL0(WaylandWindow_HideInputPanel)  // NOLINT(readability/
                                                   //         fn_size)

IPC_MESSAGE_CONTROL2(WaylandInput_Commit,  // NOLINT(readability/fn_size)
                     unsigned,
                     std::string)

IPC_MESSAGE_CONTROL3(WaylandInput_PreeditChanged, // NOLINT(readability/
                     unsigned,                    //        fn_size)
                     std::string, std::string)

IPC_MESSAGE_CONTROL0(WaylandInput_PreeditEnd)  // NOLINT(readability/fn_size)

IPC_MESSAGE_CONTROL0(WaylandInput_PreeditStart)  // NOLINT(readability/fn_size)
