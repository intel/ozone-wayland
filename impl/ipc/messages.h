// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Multiply-included message file, hence no include guard here.

#include "base/basictypes.h"
#include "base/strings/string16.h"
#include "ipc/ipc_message_macros.h"
#include "ipc/ipc_message_utils.h"
#include "ipc/ipc_param_traits.h"
#include "ipc/param_traits_macros.h"
#include "ozone/ui/events/window_constants.h"
#include "ui/events/event_constants.h"

#define IPC_MESSAGE_START LastIPCMsgStart

IPC_ENUM_TRAITS_MAX_VALUE(ui::EventFlags,
                          ui::EF_ALTGR_DOWN)
IPC_ENUM_TRAITS_MAX_VALUE(ui::EventType,
                          ui::ET_LAST)
IPC_ENUM_TRAITS_MAX_VALUE(ozonewayland::WidgetState,
                          ozonewayland::DESTROYED)
IPC_ENUM_TRAITS_MAX_VALUE(ozonewayland::WidgetType,
                          ozonewayland::POPUP)

IPC_MESSAGE_CONTROL2(WaylandInput_MotionNotify,  // NOLINT(readability/fn_size)
                     float /*x*/,
                     float /*y*/)

IPC_MESSAGE_CONTROL5(WaylandInput_ButtonNotify,  // NOLINT(readability/fn_size)
                     unsigned /*handle*/,
                     ui::EventType /*type*/,
                     ui::EventFlags /*flags*/,
                     float /*x*/,
                     float /*y*/)

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

IPC_MESSAGE_CONTROL3(WaylandInput_KeyNotify,  // NOLINT(readability/fn_size)
                     ui::EventType /*type*/,
                     unsigned /*code*/,
                     unsigned /*modifiers*/)

IPC_MESSAGE_CONTROL2(WaylandInput_OutputSize,  // NOLINT(readability/fn_size)
                     unsigned /*width*/,
                     unsigned /*height*/)

IPC_MESSAGE_CONTROL1(WaylandInput_CloseWidget,  // NOLINT(readability/fn_size)
                     unsigned /*handle*/)

IPC_MESSAGE_ROUTED4(WaylandWindow_State,  // NOLINT(readability/fn_size)
                    unsigned /* window handle */,
                    ozonewayland::WidgetState /*state*/,
                    unsigned /*width*/,
                    unsigned /*height*/)

IPC_MESSAGE_ROUTED5(WaylandWindow_Attributes,  // NOLINT(readability/fn_size)
                    unsigned /* window handle */,
                    unsigned /* window parent */,
                    unsigned /* x */,
                    unsigned /* y */,
                    ozonewayland::WidgetType /* window type */)

IPC_MESSAGE_ROUTED2(WaylandWindow_Title,  // NOLINT(readability/fn_size)
                    unsigned /* window handle */,
                    base::string16 /* window title */)
