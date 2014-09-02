/* 
 * Copyright (C) 2013 DENSO CORPORATION
 * Copyright (c) 2013 BMW Car IT GmbH
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef IVI_APPLICATION_CLIENT_PROTOCOL_H
#define IVI_APPLICATION_CLIENT_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-client.h"

struct wl_client;
struct wl_resource;

struct ivi_surface;
struct ivi_application;

extern const struct wl_interface ivi_surface_interface;
extern const struct wl_interface ivi_application_interface;

#ifndef IVI_SURFACE_WARNING_CODE_ENUM
#define IVI_SURFACE_WARNING_CODE_ENUM
/**
 * ivi_surface_warning_code - possible warning codes returned by ivi
 *	compositor
 * @IVI_SURFACE_WARNING_CODE_INVALID_WL_SURFACE: wl_surface is invalid
 * @IVI_SURFACE_WARNING_CODE_IVI_ID_IN_USE: ivi_id is in use and can not
 *	be shared
 *
 * These define all possible warning codes returned by ivi compositor on
 * server-side warnings. invalid_wl_surface: - wl_surface already has a
 * another role. - wl_surface is destroyed before the ivi_surface is
 * destroyed. ivi_id_in_use: ivi_id is already assigned by another
 * application.
 */
enum ivi_surface_warning_code {
	IVI_SURFACE_WARNING_CODE_INVALID_WL_SURFACE = 1,
	IVI_SURFACE_WARNING_CODE_IVI_ID_IN_USE = 2,
};
#endif /* IVI_SURFACE_WARNING_CODE_ENUM */

/**
 * ivi_surface - application interface to surface in ivi compositor
 * @visibility: visibility of surface in ivi compositor has changed
 * @warning: server-side warning detected
 *
 * 
 */
struct ivi_surface_listener {
	/**
	 * visibility - visibility of surface in ivi compositor has
	 *	changed
	 * @visibility: (none)
	 *
	 * The new visibility state is provided in argument visibility.
	 * If visibility is 0, the surface has become invisible. If
	 * visibility is not 0, the surface has become visible.
	 */
	void (*visibility)(void *data,
			   struct ivi_surface *ivi_surface,
			   int32_t visibility);
	/**
	 * warning - server-side warning detected
	 * @warning_code: (none)
	 * @warning_text: (none)
	 *
	 * The ivi compositor encountered warning while processing a
	 * request by this application. The warning is defined by argument
	 * warning_code and optional warning_text. If the warning is
	 * detected, client shall destroy the ivi_surface object.
	 *
	 * When a warning event is sent, the compositor turns the
	 * ivi_surface object inert. The ivi_surface will not deliver
	 * further events, all requests on it are ignored except 'destroy',
	 * and the association to the ivi_id is removed. The client should
	 * destroy the ivi_surface object. If an inert ivi_surface object
	 * is used as an argument to any other object's request, that
	 * request will [produce a fatal error / produce a warning / be
	 * ignored].
	 */
	void (*warning)(void *data,
			struct ivi_surface *ivi_surface,
			int32_t warning_code,
			const char *warning_text);
};

static inline int
ivi_surface_add_listener(struct ivi_surface *ivi_surface,
			 const struct ivi_surface_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) ivi_surface,
				     (void (**)(void)) listener, data);
}

#define IVI_SURFACE_DESTROY	0

static inline void
ivi_surface_set_user_data(struct ivi_surface *ivi_surface, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) ivi_surface, user_data);
}

static inline void *
ivi_surface_get_user_data(struct ivi_surface *ivi_surface)
{
	return wl_proxy_get_user_data((struct wl_proxy *) ivi_surface);
}

static inline void
ivi_surface_destroy(struct ivi_surface *ivi_surface)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_surface,
			 IVI_SURFACE_DESTROY);

	wl_proxy_destroy((struct wl_proxy *) ivi_surface);
}

#define IVI_APPLICATION_SURFACE_CREATE	0

static inline void
ivi_application_set_user_data(struct ivi_application *ivi_application, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) ivi_application, user_data);
}

static inline void *
ivi_application_get_user_data(struct ivi_application *ivi_application)
{
	return wl_proxy_get_user_data((struct wl_proxy *) ivi_application);
}

static inline void
ivi_application_destroy(struct ivi_application *ivi_application)
{
	wl_proxy_destroy((struct wl_proxy *) ivi_application);
}

static inline struct ivi_surface *
ivi_application_surface_create(struct ivi_application *ivi_application, uint32_t ivi_id, struct wl_surface *surface)
{
	struct wl_proxy *id;

	id = wl_proxy_marshal_constructor((struct wl_proxy *) ivi_application,
			 IVI_APPLICATION_SURFACE_CREATE, &ivi_surface_interface, ivi_id, surface, NULL);

	return (struct ivi_surface *) id;
}

#ifdef  __cplusplus
}
#endif

#endif
