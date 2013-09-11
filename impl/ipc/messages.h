// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Multiply-included message file, hence no include guard here.

#include "ipc/ipc_message_macros.h"
#include "base/basictypes.h"
#include "ipc/ipc_message_macros.h"
#include "ipc/ipc_message_utils.h"
#include "ipc/ipc_param_traits.h"
#include "ipc/param_traits_macros.h"

#define IPC_MESSAGE_START WaylandMsgStart

IPC_MESSAGE_CONTROL2(WaylandInput_MotionNotify, float /*x*/, float /*y*/)

IPC_MESSAGE_CONTROL4(WaylandInput_ButtonNotify, int /*state*/, int /*flags*/,
                     float /*x*/, float /*y*/)

IPC_MESSAGE_CONTROL4(WaylandInput_AxisNotify, float /*x*/, float /*y*/,
                     float /*x_offset*/, float /*y_offset*/)

IPC_MESSAGE_CONTROL2(WaylandInput_PointerEnter, float /*x*/, float /*y*/)

IPC_MESSAGE_CONTROL2(WaylandInput_PointerLeave, float /*x*/, float /*y*/)

IPC_MESSAGE_CONTROL3(WaylandInput_KeyNotify, unsigned /*type*/, unsigned /*code*/,
                     unsigned /*modifiers*/)

IPC_MESSAGE_CONTROL2(WaylandInput_OutputSize, unsigned /*width*/,
                     unsigned /*height*/)

// DisplayChannel in GpuProcess sends the message when it is ready to make a
// connection with DisplayChannelHost in BrowserProcess.
IPC_MESSAGE_CONTROL1(WaylandMsg_EstablishDisplayChannel, unsigned /* client id */)

// Response from DisplayChannelHost to DisplayChannel as an ack to connection
// request.
IPC_MESSAGE_ROUTED1(WaylandMsg_DisplayChannelEstablished,
                    unsigned /* channel_handle */)
