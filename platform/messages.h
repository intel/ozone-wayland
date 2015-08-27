// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Multiply-included message file, hence no include guard here.

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/memory/shared_memory.h"
#include "base/strings/string16.h"
#include "ipc/ipc_message_macros.h"
#include "ipc/ipc_message_utils.h"
#include "ipc/ipc_param_traits.h"
#include "ipc/param_traits_macros.h"
#include "ozone/platform/window_constants.h"
#include "third_party/skia/include/core/SkBitmap.h"
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
                          ui::TOOLTIP)

//------------------------------------------------------------------------------
// Browser Messages
// These messages are from the GPU to the browser process.

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

IPC_MESSAGE_CONTROL2(WaylandInput_Commit,  // NOLINT(readability/fn_size)
                     unsigned,
                     std::string)

IPC_MESSAGE_CONTROL3(WaylandInput_PreeditChanged, // NOLINT(readability/
                     unsigned,                    //        fn_size)
                     std::string, std::string)

IPC_MESSAGE_CONTROL0(WaylandInput_PreeditEnd)  // NOLINT(readability/fn_size)

IPC_MESSAGE_CONTROL0(WaylandInput_PreeditStart)  // NOLINT(readability/fn_size)

IPC_MESSAGE_CONTROL5(WaylandInput_DragEnter,  // NOLINT(readability/fn_size)
                     unsigned /* window handle */,
                     float /* x */,
                     float /* y */,
                     std::vector<std::string> /* mime_types */,
                     uint32_t /* serial */)

IPC_MESSAGE_CONTROL2(WaylandInput_DragData,  // NOLINT(readability/fn_size)
                     unsigned /* window handle */,
                     base::FileDescriptor /* pipefd */)

IPC_MESSAGE_CONTROL1(WaylandInput_DragLeave,  // NOLINT(readability/fn_size)
                     unsigned /* window handle */)

IPC_MESSAGE_CONTROL4(WaylandInput_DragMotion,  // NOLINT(readability/fn_size)
                     unsigned /* window handle */,
                     float /* x */,
                     float /* y */,
                     uint32_t /* time */)

IPC_MESSAGE_CONTROL1(WaylandInput_DragDrop,  // NOLINT(readability/fn_size)
                     unsigned /* window handle */)

//------------------------------------------------------------------------------
// GPU Messages
// These messages are from the Browser to the GPU process.

IPC_MESSAGE_CONTROL2(WaylandDisplay_State,  // NOLINT(readability/fn_size)
                     unsigned /* window handle */,
                     ui::WidgetState /*state*/)

IPC_MESSAGE_CONTROL5(WaylandDisplay_Create,  // NOLINT(readability/fn_size)
                     unsigned /* window handle */,
                     unsigned /* window parent */,
                     int /* x */,
                     int /* y */,
                     ui::WidgetType /* window type */)

IPC_MESSAGE_CONTROL4(WaylandDisplay_MoveWindow,  // NOLINT(readability/fn_size)
                     unsigned /* window handle */,
                     unsigned /* window parent */,
                     ui::WidgetType /* window type */,
                     gfx::Rect /* rect */)

IPC_MESSAGE_CONTROL2(WaylandDisplay_Title,  // NOLINT(readability/fn_size)
                     unsigned /* window handle */,
                     base::string16 /* window title */)

IPC_MESSAGE_CONTROL5(WaylandDisplay_AddRegion,  // NOLINT(readability/fn_size)
                     unsigned /* window handle */,
                     int /* left */,
                     int /* top */,
                     int /* right */,
                     int /* bottom */)

IPC_MESSAGE_CONTROL5(WaylandDisplay_SubRegion,  // NOLINT(readability/fn_size)
                     unsigned /* window handle */,
                     int /* left */,
                     int /* top */,
                     int /* right */,
                     int /* bottom */)

IPC_MESSAGE_CONTROL2(WaylandDisplay_CursorSet,  // NOLINT(readability/fn_size)
                     std::vector<SkBitmap>,
                     gfx::Point)

IPC_MESSAGE_CONTROL1(WaylandDisplay_MoveCursor,  // NOLINT(readability/fn_size)
                     gfx::Point)

IPC_MESSAGE_CONTROL0(WaylandDisplay_ImeReset)  // NOLINT(readability/fn_size)

IPC_MESSAGE_CONTROL0(WaylandDisplay_ShowInputPanel)  // NOLINT(readability/
                                                     //         fn_size)

IPC_MESSAGE_CONTROL0(WaylandDisplay_HideInputPanel)  // NOLINT(readability/
                                                     //         fn_size)

IPC_MESSAGE_CONTROL1(WaylandDisplay_RequestDragData,  // NOLINT(readability/
                     std::string /* mime_type */)     //        fn_size)

IPC_MESSAGE_CONTROL1(  // NOLINT(readability/fn_size)
    WaylandDisplay_RequestSelectionData,
    std::string /* mime_type */)

IPC_MESSAGE_CONTROL2(WaylandDisplay_DragWillBeAccepted,  // NOLINT(readability/
                     uint32_t /* serial */,              //        fn_size)
                     std::string /* mime_type */)

IPC_MESSAGE_CONTROL1(WaylandDisplay_DragWillBeRejected,  // NOLINT(readability/
                     uint32_t /* serial */)              //        fn_size)
