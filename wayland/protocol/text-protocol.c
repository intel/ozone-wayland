/*
 * Copyright © 2012, 2013 Intel Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this
 * software and its documentation for any purpose is hereby granted
 * without fee, provided that the above copyright notice appear in
 * all copies and that both that copyright notice and this permission
 * notice appear in supporting documentation, and that the name of
 * the copyright holders not be used in advertising or publicity
 * pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */

#include <stdlib.h>
#include <stdint.h>
#include "wayland-util.h"

extern const struct wl_interface wl_seat_interface;
extern const struct wl_interface wl_surface_interface;
extern const struct wl_interface wl_seat_interface;
extern const struct wl_interface wl_surface_interface;
extern const struct wl_interface wl_text_input_interface;

static const struct wl_interface *types[] = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&wl_seat_interface,
	&wl_surface_interface,
	&wl_seat_interface,
	&wl_surface_interface,
	&wl_text_input_interface,
};

static const struct wl_message wl_text_input_requests[] = {
	{ "activate", "oo", types + 5 },
	{ "deactivate", "o", types + 7 },
	{ "show_input_panel", "", types + 0 },
	{ "hide_input_panel", "", types + 0 },
	{ "reset", "", types + 0 },
	{ "set_surrounding_text", "suu", types + 0 },
	{ "set_content_type", "uu", types + 0 },
	{ "set_cursor_rectangle", "iiii", types + 0 },
	{ "set_preferred_language", "s", types + 0 },
	{ "commit_state", "u", types + 0 },
	{ "invoke_action", "uu", types + 0 },
};

static const struct wl_message wl_text_input_events[] = {
	{ "enter", "o", types + 8 },
	{ "leave", "", types + 0 },
	{ "modifiers_map", "a", types + 0 },
	{ "input_panel_state", "u", types + 0 },
	{ "preedit_string", "uss", types + 0 },
	{ "preedit_styling", "uuu", types + 0 },
	{ "preedit_cursor", "i", types + 0 },
	{ "commit_string", "us", types + 0 },
	{ "cursor_position", "ii", types + 0 },
	{ "delete_surrounding_text", "iu", types + 0 },
	{ "keysym", "uuuuu", types + 0 },
	{ "language", "us", types + 0 },
	{ "text_direction", "uu", types + 0 },
};

WL_EXPORT const struct wl_interface wl_text_input_interface = {
	"wl_text_input", 1,
	11, wl_text_input_requests,
	13, wl_text_input_events,
};

static const struct wl_message wl_text_input_manager_requests[] = {
	{ "create_text_input", "n", types + 9 },
};

WL_EXPORT const struct wl_interface wl_text_input_manager_interface = {
	"wl_text_input_manager", 1,
	1, wl_text_input_manager_requests,
	0, NULL,
};
