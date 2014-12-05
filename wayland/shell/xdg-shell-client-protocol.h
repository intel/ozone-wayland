/* 
 * Copyright © 2008-2013 Kristian Høgsberg
 * Copyright © 2013      Rafael Antognolli
 * Copyright © 2013      Jasper St. Pierre
 * Copyright © 2010-2013 Intel Corporation
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

#ifndef XDG_SHELL_CLIENT_PROTOCOL_H
#define XDG_SHELL_CLIENT_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-client.h"

struct wl_client;
struct wl_resource;

struct xdg_shell;
struct xdg_surface;
struct xdg_popup;

extern const struct wl_interface xdg_shell_interface;
extern const struct wl_interface xdg_surface_interface;
extern const struct wl_interface xdg_popup_interface;

#ifndef XDG_SHELL_VERSION_ENUM
#define XDG_SHELL_VERSION_ENUM
/**
 * xdg_shell_version - latest protocol version
 * @XDG_SHELL_VERSION_CURRENT: Always the latest version
 *
 * The 'current' member of this enum gives the version of the protocol.
 * Implementations can compare this to the version they implement using
 * static_assert to ensure the protocol and implementation versions match.
 */
enum xdg_shell_version {
	XDG_SHELL_VERSION_CURRENT = 4,
};
#endif /* XDG_SHELL_VERSION_ENUM */

/**
 * xdg_shell - create desktop-style surfaces
 * @ping: check if the client is alive
 *
 * This interface is implemented by servers that provide desktop-style
 * user interfaces.
 *
 * It allows clients to associate a xdg_surface with a basic surface.
 */
struct xdg_shell_listener {
	/**
	 * ping - check if the client is alive
	 * @serial: pass this to the callback
	 *
	 * The ping event asks the client if it's still alive. Pass the
	 * serial specified in the event back to the compositor by sending
	 * a "pong" request back with the specified serial.
	 *
	 * Compositors can use this to determine if the client is still
	 * alive. It's unspecified what will happen if the client doesn't
	 * respond to the ping request, or in what timeframe. Clients
	 * should try to respond in a reasonable amount of time.
	 */
	void (*ping)(void *data,
		     struct xdg_shell *xdg_shell,
		     uint32_t serial);
};

static inline int
xdg_shell_add_listener(struct xdg_shell *xdg_shell,
		       const struct xdg_shell_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) xdg_shell,
				     (void (**)(void)) listener, data);
}

#define XDG_SHELL_USE_UNSTABLE_VERSION	0
#define XDG_SHELL_GET_XDG_SURFACE	1
#define XDG_SHELL_GET_XDG_POPUP	2
#define XDG_SHELL_PONG	3

static inline void
xdg_shell_set_user_data(struct xdg_shell *xdg_shell, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) xdg_shell, user_data);
}

static inline void *
xdg_shell_get_user_data(struct xdg_shell *xdg_shell)
{
	return wl_proxy_get_user_data((struct wl_proxy *) xdg_shell);
}

static inline void
xdg_shell_destroy(struct xdg_shell *xdg_shell)
{
	wl_proxy_destroy((struct wl_proxy *) xdg_shell);
}

static inline void
xdg_shell_use_unstable_version(struct xdg_shell *xdg_shell, int32_t version)
{
	wl_proxy_marshal((struct wl_proxy *) xdg_shell,
			 XDG_SHELL_USE_UNSTABLE_VERSION, version);
}

static inline struct xdg_surface *
xdg_shell_get_xdg_surface(struct xdg_shell *xdg_shell, struct wl_surface *surface)
{
	struct wl_proxy *id;

	id = wl_proxy_marshal_constructor((struct wl_proxy *) xdg_shell,
			 XDG_SHELL_GET_XDG_SURFACE, &xdg_surface_interface, NULL, surface);

	return (struct xdg_surface *) id;
}

static inline struct xdg_popup *
xdg_shell_get_xdg_popup(struct xdg_shell *xdg_shell, struct wl_surface *surface, struct wl_surface *parent, struct wl_seat *seat, uint32_t serial, int32_t x, int32_t y, uint32_t flags)
{
	struct wl_proxy *id;

	id = wl_proxy_marshal_constructor((struct wl_proxy *) xdg_shell,
			 XDG_SHELL_GET_XDG_POPUP, &xdg_popup_interface, NULL, surface, parent, seat, serial, x, y, flags);

	return (struct xdg_popup *) id;
}

static inline void
xdg_shell_pong(struct xdg_shell *xdg_shell, uint32_t serial)
{
	wl_proxy_marshal((struct wl_proxy *) xdg_shell,
			 XDG_SHELL_PONG, serial);
}

#ifndef XDG_SURFACE_RESIZE_EDGE_ENUM
#define XDG_SURFACE_RESIZE_EDGE_ENUM
/**
 * xdg_surface_resize_edge - edge values for resizing
 * @XDG_SURFACE_RESIZE_EDGE_NONE: (none)
 * @XDG_SURFACE_RESIZE_EDGE_TOP: (none)
 * @XDG_SURFACE_RESIZE_EDGE_BOTTOM: (none)
 * @XDG_SURFACE_RESIZE_EDGE_LEFT: (none)
 * @XDG_SURFACE_RESIZE_EDGE_TOP_LEFT: (none)
 * @XDG_SURFACE_RESIZE_EDGE_BOTTOM_LEFT: (none)
 * @XDG_SURFACE_RESIZE_EDGE_RIGHT: (none)
 * @XDG_SURFACE_RESIZE_EDGE_TOP_RIGHT: (none)
 * @XDG_SURFACE_RESIZE_EDGE_BOTTOM_RIGHT: (none)
 *
 * These values are used to indicate which edge of a surface is being
 * dragged in a resize operation. The server may use this information to
 * adapt its behavior, e.g. choose an appropriate cursor image.
 */
enum xdg_surface_resize_edge {
	XDG_SURFACE_RESIZE_EDGE_NONE = 0,
	XDG_SURFACE_RESIZE_EDGE_TOP = 1,
	XDG_SURFACE_RESIZE_EDGE_BOTTOM = 2,
	XDG_SURFACE_RESIZE_EDGE_LEFT = 4,
	XDG_SURFACE_RESIZE_EDGE_TOP_LEFT = 5,
	XDG_SURFACE_RESIZE_EDGE_BOTTOM_LEFT = 6,
	XDG_SURFACE_RESIZE_EDGE_RIGHT = 8,
	XDG_SURFACE_RESIZE_EDGE_TOP_RIGHT = 9,
	XDG_SURFACE_RESIZE_EDGE_BOTTOM_RIGHT = 10,
};
#endif /* XDG_SURFACE_RESIZE_EDGE_ENUM */

#ifndef XDG_SURFACE_STATE_ENUM
#define XDG_SURFACE_STATE_ENUM
/**
 * xdg_surface_state - types of state on the surface
 * @XDG_SURFACE_STATE_MAXIMIZED: the surface is maximized
 * @XDG_SURFACE_STATE_FULLSCREEN: the surface is fullscreen
 * @XDG_SURFACE_STATE_RESIZING: (none)
 * @XDG_SURFACE_STATE_ACTIVATED: (none)
 *
 * The different state values used on the surface. This is designed for
 * state values like maximized, fullscreen. It is paired with the configure
 * event to ensure that both the client and the compositor setting the
 * state can be synchronized.
 *
 * States set in this way are double-buffered. They will get applied on the
 * next commit.
 *
 * Desktop environments may extend this enum by taking up a range of values
 * and documenting the range they chose in this description. They are not
 * required to document the values for the range that they chose. Ideally,
 * any good extensions from a desktop environment should make its way into
 * standardization into this enum.
 *
 * The current reserved ranges are:
 *
 * 0x0000 - 0x0FFF: xdg-shell core values, documented below. 0x1000 -
 * 0x1FFF: GNOME
 */
enum xdg_surface_state {
	XDG_SURFACE_STATE_MAXIMIZED = 1,
	XDG_SURFACE_STATE_FULLSCREEN = 2,
	XDG_SURFACE_STATE_RESIZING = 3,
	XDG_SURFACE_STATE_ACTIVATED = 4,
};
#endif /* XDG_SURFACE_STATE_ENUM */

/**
 * xdg_surface - desktop-style metadata interface
 * @configure: suggest a surface change
 * @close: surface wants to be closed
 *
 * An interface that may be implemented by a wl_surface, for
 * implementations that provide a desktop-style user interface.
 *
 * It provides requests to treat surfaces like windows, allowing to set
 * properties like maximized, fullscreen, minimized, and to move and resize
 * them, and associate metadata like title and app id.
 *
 * On the server side the object is automatically destroyed when the
 * related wl_surface is destroyed. On client side, xdg_surface.destroy()
 * must be called before destroying the wl_surface object.
 */
struct xdg_surface_listener {
	/**
	 * configure - suggest a surface change
	 * @width: (none)
	 * @height: (none)
	 * @states: (none)
	 * @serial: (none)
	 *
	 * The configure event asks the client to resize its surface.
	 *
	 * The width and height arguments specify a hint to the window
	 * about how its surface should be resized in window geometry
	 * coordinates. The states listed in the event specify how the
	 * width/height arguments should be interpreted.
	 *
	 * A client should arrange a new surface, and then send a
	 * ack_configure request with the serial sent in this configure
	 * event before attaching a new surface.
	 *
	 * If the client receives multiple configure events before it can
	 * respond to one, it is free to discard all but the last event it
	 * received.
	 */
	void (*configure)(void *data,
			  struct xdg_surface *xdg_surface,
			  int32_t width,
			  int32_t height,
			  struct wl_array *states,
			  uint32_t serial);
	/**
	 * close - surface wants to be closed
	 *
	 * The close event is sent by the compositor when the user wants
	 * the surface to be closed. This should be equivalent to the user
	 * clicking the close button in client-side decorations, if your
	 * application has any...
	 *
	 * This is only a request that the user intends to close your
	 * window. The client may choose to ignore this request, or show a
	 * dialog to ask the user to save their data...
	 */
	void (*close)(void *data,
		      struct xdg_surface *xdg_surface);
};

static inline int
xdg_surface_add_listener(struct xdg_surface *xdg_surface,
			 const struct xdg_surface_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) xdg_surface,
				     (void (**)(void)) listener, data);
}

#define XDG_SURFACE_DESTROY	0
#define XDG_SURFACE_SET_PARENT	1
#define XDG_SURFACE_SET_TITLE	2
#define XDG_SURFACE_SET_APP_ID	3
#define XDG_SURFACE_SHOW_WINDOW_MENU	4
#define XDG_SURFACE_MOVE	5
#define XDG_SURFACE_RESIZE	6
#define XDG_SURFACE_ACK_CONFIGURE	7
#define XDG_SURFACE_SET_WINDOW_GEOMETRY	8
#define XDG_SURFACE_SET_MAXIMIZED	9
#define XDG_SURFACE_UNSET_MAXIMIZED	10
#define XDG_SURFACE_SET_FULLSCREEN	11
#define XDG_SURFACE_UNSET_FULLSCREEN	12
#define XDG_SURFACE_SET_MINIMIZED	13

static inline void
xdg_surface_set_user_data(struct xdg_surface *xdg_surface, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) xdg_surface, user_data);
}

static inline void *
xdg_surface_get_user_data(struct xdg_surface *xdg_surface)
{
	return wl_proxy_get_user_data((struct wl_proxy *) xdg_surface);
}

static inline void
xdg_surface_destroy(struct xdg_surface *xdg_surface)
{
	wl_proxy_marshal((struct wl_proxy *) xdg_surface,
			 XDG_SURFACE_DESTROY);

	wl_proxy_destroy((struct wl_proxy *) xdg_surface);
}

static inline void
xdg_surface_set_parent(struct xdg_surface *xdg_surface, struct wl_surface *parent)
{
	wl_proxy_marshal((struct wl_proxy *) xdg_surface,
			 XDG_SURFACE_SET_PARENT, parent);
}

static inline void
xdg_surface_set_title(struct xdg_surface *xdg_surface, const char *title)
{
	wl_proxy_marshal((struct wl_proxy *) xdg_surface,
			 XDG_SURFACE_SET_TITLE, title);
}

static inline void
xdg_surface_set_app_id(struct xdg_surface *xdg_surface, const char *app_id)
{
	wl_proxy_marshal((struct wl_proxy *) xdg_surface,
			 XDG_SURFACE_SET_APP_ID, app_id);
}

static inline void
xdg_surface_show_window_menu(struct xdg_surface *xdg_surface, struct wl_seat *seat, uint32_t serial, int32_t x, int32_t y)
{
	wl_proxy_marshal((struct wl_proxy *) xdg_surface,
			 XDG_SURFACE_SHOW_WINDOW_MENU, seat, serial, x, y);
}

static inline void
xdg_surface_move(struct xdg_surface *xdg_surface, struct wl_seat *seat, uint32_t serial)
{
	wl_proxy_marshal((struct wl_proxy *) xdg_surface,
			 XDG_SURFACE_MOVE, seat, serial);
}

static inline void
xdg_surface_resize(struct xdg_surface *xdg_surface, struct wl_seat *seat, uint32_t serial, uint32_t edges)
{
	wl_proxy_marshal((struct wl_proxy *) xdg_surface,
			 XDG_SURFACE_RESIZE, seat, serial, edges);
}

static inline void
xdg_surface_ack_configure(struct xdg_surface *xdg_surface, uint32_t serial)
{
	wl_proxy_marshal((struct wl_proxy *) xdg_surface,
			 XDG_SURFACE_ACK_CONFIGURE, serial);
}

static inline void
xdg_surface_set_window_geometry(struct xdg_surface *xdg_surface, int32_t x, int32_t y, int32_t width, int32_t height)
{
	wl_proxy_marshal((struct wl_proxy *) xdg_surface,
			 XDG_SURFACE_SET_WINDOW_GEOMETRY, x, y, width, height);
}

static inline void
xdg_surface_set_maximized(struct xdg_surface *xdg_surface)
{
	wl_proxy_marshal((struct wl_proxy *) xdg_surface,
			 XDG_SURFACE_SET_MAXIMIZED);
}

static inline void
xdg_surface_unset_maximized(struct xdg_surface *xdg_surface)
{
	wl_proxy_marshal((struct wl_proxy *) xdg_surface,
			 XDG_SURFACE_UNSET_MAXIMIZED);
}

static inline void
xdg_surface_set_fullscreen(struct xdg_surface *xdg_surface, struct wl_output *output)
{
	wl_proxy_marshal((struct wl_proxy *) xdg_surface,
			 XDG_SURFACE_SET_FULLSCREEN, output);
}

static inline void
xdg_surface_unset_fullscreen(struct xdg_surface *xdg_surface)
{
	wl_proxy_marshal((struct wl_proxy *) xdg_surface,
			 XDG_SURFACE_UNSET_FULLSCREEN);
}

static inline void
xdg_surface_set_minimized(struct xdg_surface *xdg_surface)
{
	wl_proxy_marshal((struct wl_proxy *) xdg_surface,
			 XDG_SURFACE_SET_MINIMIZED);
}

/**
 * xdg_popup - desktop-style metadata interface
 * @popup_done: popup interaction is done
 *
 * An interface that may be implemented by a wl_surface, for
 * implementations that provide a desktop-style popups/menus. A popup
 * surface is a transient surface with an added pointer grab.
 *
 * An existing implicit grab will be changed to owner-events mode, and the
 * popup grab will continue after the implicit grab ends (i.e. releasing
 * the mouse button does not cause the popup to be unmapped).
 *
 * The popup grab continues until the window is destroyed or a mouse button
 * is pressed in any other clients window. A click in any of the clients
 * surfaces is reported as normal, however, clicks in other clients
 * surfaces will be discarded and trigger the callback.
 *
 * The x and y arguments specify the locations of the upper left corner of
 * the surface relative to the upper left corner of the parent surface, in
 * surface local coordinates.
 *
 * xdg_popup surfaces are always transient for another surface.
 */
struct xdg_popup_listener {
	/**
	 * popup_done - popup interaction is done
	 * @serial: serial of the implicit grab on the pointer
	 *
	 * The popup_done event is sent out when a popup grab is broken,
	 * that is, when the users clicks a surface that doesn't belong to
	 * the client owning the popup surface.
	 */
	void (*popup_done)(void *data,
			   struct xdg_popup *xdg_popup,
			   uint32_t serial);
};

static inline int
xdg_popup_add_listener(struct xdg_popup *xdg_popup,
		       const struct xdg_popup_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) xdg_popup,
				     (void (**)(void)) listener, data);
}

#define XDG_POPUP_DESTROY	0

static inline void
xdg_popup_set_user_data(struct xdg_popup *xdg_popup, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) xdg_popup, user_data);
}

static inline void *
xdg_popup_get_user_data(struct xdg_popup *xdg_popup)
{
	return wl_proxy_get_user_data((struct wl_proxy *) xdg_popup);
}

static inline void
xdg_popup_destroy(struct xdg_popup *xdg_popup)
{
	wl_proxy_marshal((struct wl_proxy *) xdg_popup,
			 XDG_POPUP_DESTROY);

	wl_proxy_destroy((struct wl_proxy *) xdg_popup);
}

#ifdef  __cplusplus
}
#endif

#endif
