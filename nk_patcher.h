 /*
 * Copyright (c) 2016 Hanspeter Portner (dev@open-music-kontrollers.ch)
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the Artistic License 2.0 as published by
 * The Perl Foundation.
 *
 * This src is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the iapplied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * Artistic License 2.0 for more details.
 *
 * You should have received a copy of the Artistic License 2.0
 * along the src as a COPYING file. If not, obtain it from
 * http://www.perlfoundation.org/artistic_license_2_0.
 */

#ifndef _NK_PATCHER_H
#define _NK_PATCHER_H

typedef enum _nk_patcher_type_t nk_patcher_type_t;
typedef struct _nk_patcher_port_t nk_patcher_port_t;
typedef struct _nk_patcher_connection_t nk_patcher_connection_t;
typedef struct _nk_patcher_t nk_patcher_t;
typedef void (*nk_patcher_fill_t)(void *data, uintptr_t src_id, uintptr_t snk_id,
	bool *state, nk_patcher_type_t *type);
typedef void (nk_patcher_change_t)(void *data, uintptr_t src_id, uintptr_t snk_id,
	bool state);

enum _nk_patcher_type_t {
	NK_PATCHER_TYPE_DIRECT,
	NK_PATCHER_TYPE_FEEDBACK,
	NK_PATCHER_TYPE_INDIRECT
};

struct _nk_patcher_port_t {
	int idx;
	uintptr_t id;
	struct nk_color color;
	char *label;
};

struct _nk_patcher_connection_t {
	bool state;
	nk_patcher_type_t type;
	int enm;
};

struct _nk_patcher_t {
	int src_n;
	int snk_n;
	float scale;

	nk_patcher_port_t *srcs;
	nk_patcher_port_t *snks;

	nk_patcher_connection_t **connections;

	// cached 'constants'
	float X, Y;
	float W, H;
	float W2, H2;
	float span, span1, span2;
	float x0, y0;
};

enum {
	CONNECTED				= (1 << 0),
	VERTICAL				= (1 << 1),
	HORIZONTAL			= (1 << 2),
	VERTICAL_EDGE		= (1 << 3),
	HORIZONTAL_EDGE	= (1 << 4),
	FEEDBACK				= (1 << 5),
	INDIRECT				= (1 << 6)
};

static void
nk_patcher_init(nk_patcher_t *patch, float scale);

static void
nk_patcher_reinit(nk_patcher_t *patch, int src_n, int snk_n);

static void
nk_patcher_deinit(nk_patcher_t *patch);

static int
nk_patcher_connected_set(nk_patcher_t *patch, uintptr_t src_id, intptr_t snk_id,
	bool state, nk_patcher_type_t type);

static int
nk_patcher_src_id_set(nk_patcher_t *patch, int src_idx, uintptr_t src_id);

static int
nk_patcher_snk_id_set(nk_patcher_t *patch, int snk_idx, uintptr_t snk_id);

static int
nk_patcher_src_color_set(nk_patcher_t *patch, int src_idx, struct nk_color src_color);

static int
nk_patcher_snk_color_set(nk_patcher_t *patch, int snk_idx, struct nk_color snk_color);

static int
nk_patcher_src_label_set(nk_patcher_t *patch, int src_idx, const char *src_label);

static int
nk_patcher_snk_label_set(nk_patcher_t *patch, int snk_idx, const char *snk_label);

static int
nk_patcher_src_group_set(nk_patcher_t *patch, int src_idx, const char *src_group);

static int
nk_patcher_snk_group_set(nk_patcher_t *patch, int snk_idx, const char *snk_group);

static void
nk_patcher_render(nk_patcher_t *patch, struct nk_context *ctx, struct nk_rect bounds,
	nk_patcher_change_t *change, void *data);

static void
nk_patcher_fill(nk_patcher_t *patch, nk_patcher_fill_t fill, void *data);

#endif // _NK_PATCHER_H

#ifdef NK_PATCHER_IMPLEMENTATION
static const struct nk_color bright = {.r = 0xee, .g = 0xee, .b = 0xee, .a = 0xff};

static void
nk_patcher_init(nk_patcher_t *patch, float scale)
{
	nk_patcher_reinit(patch, 0, 0);

	patch->scale = scale;
}

static void
nk_patcher_reinit(nk_patcher_t *patch, int src_n, int snk_n)
{
	nk_patcher_deinit(patch);

	patch->src_n = src_n;
	patch->snk_n = snk_n;

	patch->srcs = patch->src_n ? calloc(patch->src_n, sizeof(nk_patcher_port_t)) : NULL;
	patch->snks = patch->snk_n ? calloc(patch->snk_n, sizeof(nk_patcher_port_t)) : NULL;

	patch->connections = patch->src_n ? calloc(patch->src_n, sizeof(nk_patcher_connection_t *)) : NULL;
	for(int src_idx=0; src_idx<patch->src_n; src_idx++)
	{
		patch->connections[src_idx] = snk_n ? calloc(patch->snk_n, sizeof(nk_patcher_connection_t)) : NULL;
	}
}

static void
nk_patcher_deinit(nk_patcher_t *patch)
{
	if(patch->connections)
	{
		for(int src_idx=0; src_idx<patch->src_n; src_idx++)
		{
			if(patch->connections[src_idx])
				free(patch->connections[src_idx]);
		}
		free(patch->connections);
	}

	if(patch->srcs)
	{
		for(int src_idx=0; src_idx<patch->src_n; src_idx++)
		{
			nk_patcher_port_t *port = &patch->srcs[src_idx];

			if(port->label)
				free(port->label);
		}
		free(patch->srcs);
	}

	if(patch->snks)
	{
		for(int snk_idx=0; snk_idx<patch->snk_n; snk_idx++)
		{
			nk_patcher_port_t *port = &patch->snks[snk_idx];

			if(port->label)
				free(port->label);
		}
		free(patch->snks);
	}

	patch->src_n = 0;
	patch->snk_n = 0;
}

static inline int
_nk_patcher_src_idx_get(nk_patcher_t *patch, uintptr_t src_id)
{
	for(int src_idx=0; src_idx<patch->src_n; src_idx++)
	{
		if(patch->srcs[src_idx].id == src_id)
			return src_idx;
	}

	return -1; // not found
}

static inline int
_nk_patcher_snk_idx_get(nk_patcher_t *patch, uintptr_t snk_id)
{
	for(int snk_idx=0; snk_idx<patch->snk_n; snk_idx++)
	{
		if(patch->snks[snk_idx].id == snk_id)
			return snk_idx;
	}

	return -1; // not found
}

static int
nk_patcher_connected_set(nk_patcher_t *patch, uintptr_t src_id, intptr_t snk_id,
	bool state, nk_patcher_type_t type)
{
	const int src_idx = _nk_patcher_src_idx_get(patch, src_id);
	const int snk_idx = _nk_patcher_snk_idx_get(patch, snk_id);

	if( (src_idx == -1) || (snk_idx == -1) )
		return -1;

	nk_patcher_connection_t *conn = &patch->connections[src_idx][snk_idx];
	conn->state = state;
	conn->type = type;

	return 0;
}

static int
nk_patcher_src_id_set(nk_patcher_t *patch, int src_idx, uintptr_t src_id)
{
	if( (src_idx < 0) || (src_idx >= patch->src_n) )
		return -1;

	nk_patcher_port_t *port = &patch->srcs[src_idx];
	port->id = src_id;

	return 0;
}

static int
nk_patcher_snk_id_set(nk_patcher_t *patch, int snk_idx, uintptr_t snk_id)
{
	if( (snk_idx < 0) || (snk_idx >= patch->snk_n) )
		return -1;

	nk_patcher_port_t *port = &patch->snks[snk_idx];
	port->id = snk_id;

	return 0;
}

static int
nk_patcher_src_color_set(nk_patcher_t *patch, int src_idx, struct nk_color src_color)
{
	if( (src_idx < 0) || (src_idx >= patch->src_n) )
		return -1;

	nk_patcher_port_t *port = &patch->srcs[src_idx];
	port->color = src_color;

	return 0;
}

static int
nk_patcher_snk_color_set(nk_patcher_t *patch, int snk_idx, struct nk_color snk_color)
{
	if( (snk_idx < 0) || (snk_idx >= patch->snk_n) )
		return -1;

	nk_patcher_port_t *port = &patch->snks[snk_idx];
	port->color = snk_color;

	return 0;
}

static int
nk_patcher_src_label_set(nk_patcher_t *patch, int src_idx, const char *src_label)
{
	if( (src_idx < 0) || (src_idx >= patch->src_n) )
		return -1;

	nk_patcher_port_t *port = &patch->srcs[src_idx];
	if(port->label)
		free(port->label);
	port->label = strdup(src_label);

	return 0;
}

static int
nk_patcher_snk_label_set(nk_patcher_t *patch, int snk_idx, const char *snk_label)
{
	if( (snk_idx < 0) || (snk_idx >= patch->snk_n) )
		return -1;

	nk_patcher_port_t *port = &patch->snks[snk_idx];
	if(port->label)
		free(port->label);
	port->label = strdup(snk_label);

	return 0;
}

static inline void
_rel_to_abs(nk_patcher_t *patch, float ax, float ay, float *_fx, float *_fy)
{
	ay = patch->snk_n - ay;
	float fx = patch->x0 + patch->span * ( ax + ay);
	float fy = patch->y0 + patch->span * (-ax + ay);

	fx =  fx*patch->W2 + patch->X + patch->W2;
	fy = -fy*patch->H2 + patch->Y + patch->H2;

	*_fx = fx;
	*_fy = fy;
}

static inline void
_abs_to_rel(nk_patcher_t *patch, float fx, float fy, int *_ax, int *_ay)
{
	fx =  (fx - patch->X - patch->W2) / patch->W2;
	fy = -(fy - patch->Y - patch->H2) / patch->H2;

	float ax = floor( (-patch->x0 + fx + patch->y0 - fy) * patch->span2);
	float ay = floor( (-patch->x0 + fx - patch->y0 + fy) * patch->span2);
	ay = patch->snk_n - 1 - ay;

	if( (ax >= 0) && (ax < patch->src_n) && (ay >= 0) && (ay < patch->snk_n) )
	{
		// inside-bounds
	}
	else if(ax >= patch->src_n)
	{
		ax = -1; // out-of-bounds

		ay = floor( (-patch->y0 - fy) * patch->span1);
		if( (ay < 0) || (ay >= patch->snk_n) )
			ay = -1; // out-of-bounds
	}
	else if(ay >= patch->snk_n)
	{
		ax = floor( (patch->y0 - fy) * patch->span1);
		if( (ax < 0) || (ax >= patch->src_n) )
			ax = -1; // out-of-bounds

		ay = -1; // out-of-bounds
	}
	else
	{
		ax = -1; // out-of-bounds
		ay = -1; // out-of-bounds
	}

	*_ax = ax;
	*_ay = ay;
}

static void
_precalc(nk_patcher_t *patch, struct nk_rect bounds)
{
	if(patch->src_n > patch->snk_n)
	{
		patch->span = 1.f*patch->scale / patch->src_n;
		const float offset = patch->span * (patch->src_n - patch->snk_n) * 0.5;
		patch->x0 = -1.f*patch->scale + offset;
		patch->y0 =  0.f*patch->scale + offset;
	}
	else if(patch->snk_n > patch->src_n)
	{
		patch->span = 1.f*patch->scale / patch->snk_n;
		const float offset = patch->span * (patch->snk_n - patch->src_n) * 0.5;
		patch->x0 = -1.f*patch->scale + offset;
		patch->y0 =  0.f*patch->scale - offset;
	}
	else // patch->snk_n == patch->src_n
	{
		patch->span = 1.f*patch->scale / patch->src_n;
		patch->x0 = -1.f*patch->scale;
		patch->y0 =  0.f*patch->scale;
	}

	patch->span1 = 1.f / patch->span;
	patch->span2 = 0.5 / patch->span;

	patch->W = bounds.w > bounds.h ? bounds.w : bounds.h;
	patch->H = bounds.h > bounds.w ? bounds.h : bounds.w;

	patch->W2 = patch->W / 2.f;
	patch->H2 = patch->H / 2.f;

	patch->X = bounds.x - (patch->W - bounds.w) / 2;
	patch->Y = bounds.y - (patch->H - bounds.h) / 2;
}

static void
nk_patcher_render(nk_patcher_t *patch, struct nk_context *ctx, struct nk_rect bounds,
	nk_patcher_change_t *change, void *data)
{
	if(  patch->src_n && patch->snk_n
		&& (nk_widget(&bounds, ctx) != NK_WIDGET_INVALID) )
	{
		struct nk_style *style = &ctx->style;
		struct nk_input *in = &ctx->input;
		int src_ptr = -1; // initialize
		int snk_ptr = -1; // initialize

		_precalc(patch, bounds);

		// handle mouse input
		if(nk_input_is_mouse_hovering_rect(in, bounds))
		{
			if(in->mouse.scroll_delta)
			{
				patch->scale *= 1.0 + in->mouse.scroll_delta * 0.05;
				patch->scale = NK_CLAMP(0.1, patch->scale, 0.8);
				_precalc(patch, bounds);

				in->mouse.scroll_delta = 0.f;
			}

			_abs_to_rel(patch, in->mouse.pos.x, in->mouse.pos.y, &src_ptr, &snk_ptr);

			// handle state toggling
			if(change && nk_input_is_mouse_pressed(in, NK_BUTTON_LEFT))
			{
				if( (src_ptr != -1) && (snk_ptr != -1) )
				{
					nk_patcher_port_t *src_port = &patch->srcs[src_ptr];
					nk_patcher_port_t *snk_port = &patch->snks[snk_ptr];
					nk_patcher_connection_t *conn = &patch->connections[src_ptr][snk_ptr];

					change(data, src_port->id, snk_port->id, !conn->state);
				}
				else if(src_ptr != -1)
				{
					nk_patcher_port_t *src_port = &patch->srcs[src_ptr];
					bool state = false;

					for(int snk_idx = 0; snk_idx < patch->snk_n; snk_idx++)
					{
						nk_patcher_connection_t *conn = &patch->connections[src_ptr][snk_idx];

						state = state || conn->state;
					}
					for(int snk_idx = 0; snk_idx < patch->snk_n; snk_idx++)
					{
						nk_patcher_port_t *snk_port = &patch->snks[snk_idx];

						change(data, src_port->id, snk_port->id, !state);
					}
				}
				else if(snk_ptr != -1)
				{
					nk_patcher_port_t *snk_port = &patch->snks[snk_ptr];
					bool state = false;

					for(int src_idx = 0; src_idx < patch->src_n; src_idx++)
					{
						nk_patcher_connection_t *conn = &patch->connections[src_idx][snk_ptr];

						state = state || conn->state;
					}
					for(int src_idx = 0; src_idx < patch->src_n; src_idx++)
					{
						nk_patcher_port_t *src_port = &patch->srcs[src_idx];

						change(data, src_port->id, snk_port->id, !state);
					}
				}
			}
		}

		// reset patch fields
		for(int src_idx = 0; src_idx < patch->src_n; src_idx++)
		{
			for(int snk_idx = 0; snk_idx < patch->snk_n; snk_idx++)
			{
				nk_patcher_connection_t *conn = &patch->connections[src_idx][snk_idx];

				conn->enm = 0;
				switch(conn->type)
				{
					case NK_PATCHER_TYPE_DIRECT:
						break;
					case NK_PATCHER_TYPE_FEEDBACK:
						conn->enm |= FEEDBACK;
						break;
					case NK_PATCHER_TYPE_INDIRECT:
						conn->enm |= INDIRECT;
						break;
				}
			}
		}

		// fill patch fields
		if( (src_ptr != -1) && (snk_ptr != -1) )
		{
			for(int src_idx = 0; src_idx < patch->src_n; src_idx++)
			{
				for(int snk_idx = 0; snk_idx < patch->snk_n; snk_idx++)
				{
					nk_patcher_connection_t *conn = &patch->connections[src_idx][snk_idx];

					if( (snk_idx == snk_ptr) && (src_idx >  src_ptr) )
						conn->enm |= HORIZONTAL;
					if( (snk_idx == snk_ptr) && (src_idx == src_ptr) )
						conn->enm |= HORIZONTAL_EDGE | VERTICAL_EDGE;
					if( (snk_idx >  snk_ptr) && (src_idx == src_ptr) )
						conn->enm |= VERTICAL;
				}
			}
		}
		else if(src_ptr != -1)
		{
			int thresh = patch->snk_n;
			for(int snk_idx = thresh-1; snk_idx >= 0; snk_idx--)
			{
				if(patch->connections[src_ptr][snk_idx].state)
				{
					thresh = snk_idx;

					patch->connections[src_ptr][snk_idx].enm |= HORIZONTAL_EDGE;
					for(int src_idx = src_ptr+1; src_idx < patch->src_n; src_idx++)
						patch->connections[src_idx][snk_idx].enm |= HORIZONTAL;
				}
			}

			for(int snk_idx = thresh; snk_idx < patch->snk_n; snk_idx++)
				patch->connections[src_ptr][snk_idx].enm |= snk_idx == thresh ? VERTICAL_EDGE : VERTICAL;
		}
		else if(snk_ptr != -1)
		{
			int thresh = patch->src_n;
			for(int src_idx = thresh-1; src_idx >= 0; src_idx--)
			{
				if(patch->connections[src_idx][snk_ptr].state)
				{
					thresh = src_idx;

					patch->connections[src_idx][snk_ptr].enm |= VERTICAL_EDGE;
					for(int snk_idx = snk_ptr+1; snk_idx < patch->snk_n; snk_idx++)
						patch->connections[src_idx][snk_idx].enm |= VERTICAL;
				}
			}

			for(int src_idx = thresh; src_idx < patch->src_n; src_idx++)
				patch->connections[src_idx][snk_ptr].enm |= src_idx == thresh ? HORIZONTAL_EDGE : HORIZONTAL;
		}

		struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);

		// draw patch fields
		for(int src_idx = 0; src_idx < patch->src_n; src_idx++)
		{
			for(int snk_idx = 0; snk_idx < patch->snk_n; snk_idx++)
			{
				nk_patcher_connection_t *conn = &patch->connections[src_idx][snk_idx];
				float p [8];

				// FEEDBACK | INDIRECT | DIRECT
				_rel_to_abs(patch, src_idx + 0.0, snk_idx + 0.0, &p[6], &p[7]);
				_rel_to_abs(patch, src_idx + 0.0, snk_idx + 1.0, &p[4], &p[5]);
				_rel_to_abs(patch, src_idx + 1.0, snk_idx + 1.0, &p[2], &p[3]);
				_rel_to_abs(patch, src_idx + 1.0, snk_idx + 0.0, &p[0], &p[1]);
				if(conn->enm & FEEDBACK)
					nk_fill_polygon(canvas, p, 4, style->button.hover.data.color);
				else if(conn->enm & INDIRECT)
					nk_fill_polygon(canvas, p, 4, style->button.active.data.color);
				else
					nk_fill_polygon(canvas, p, 4, style->button.normal.data.color);

				// HORIZONTAL
				if(conn->enm & HORIZONTAL)
				{
					_rel_to_abs(patch, src_idx + 0.0, snk_idx + 0.4, &p[6], &p[7]);
					_rel_to_abs(patch, src_idx + 0.0, snk_idx + 0.6, &p[4], &p[5]);
					_rel_to_abs(patch, src_idx + 1.0, snk_idx + 0.6, &p[2], &p[3]);
					_rel_to_abs(patch, src_idx + 1.0, snk_idx + 0.4, &p[0], &p[1]);

					nk_fill_polygon(canvas, p, 4, patch->snks[snk_idx].color);
				}

				// HORIZONTAL_EDGE
				if(conn->enm & HORIZONTAL_EDGE)
				{
					_rel_to_abs(patch, src_idx + 0.6, snk_idx + 0.4, &p[6], &p[7]);
					_rel_to_abs(patch, src_idx + 0.6, snk_idx + 0.6, &p[4], &p[5]);
					_rel_to_abs(patch, src_idx + 1.0, snk_idx + 0.6, &p[2], &p[3]);
					_rel_to_abs(patch, src_idx + 1.0, snk_idx + 0.4, &p[0], &p[1]);

					nk_fill_polygon(canvas, p, 4, patch->snks[snk_idx].color);
				}

				// VERTICAL
				if(conn->enm & VERTICAL)
				{
					_rel_to_abs(patch, src_idx + 0.4, snk_idx + 0.0, &p[6], &p[7]);
					_rel_to_abs(patch, src_idx + 0.4, snk_idx + 1.0, &p[4], &p[5]);
					_rel_to_abs(patch, src_idx + 0.6, snk_idx + 1.0, &p[2], &p[3]);
					_rel_to_abs(patch, src_idx + 0.6, snk_idx + 0.0, &p[0], &p[1]);

					nk_fill_polygon(canvas, p, 4, patch->srcs[src_idx].color);
				}

				// VERTICAL_EDGE
				if(conn->enm & VERTICAL_EDGE)
				{
					_rel_to_abs(patch, src_idx + 0.4, snk_idx + 0.6, &p[6], &p[7]);
					_rel_to_abs(patch, src_idx + 0.4, snk_idx + 1.0, &p[4], &p[5]);
					_rel_to_abs(patch, src_idx + 0.6, snk_idx + 1.0, &p[2], &p[3]);
					_rel_to_abs(patch, src_idx + 0.6, snk_idx + 0.6, &p[0], &p[1]);

					nk_fill_polygon(canvas, p, 4, patch->srcs[src_idx].color);
				}

				// CONNECTED
				if(conn->state)
				{
					_rel_to_abs(patch, src_idx + 0.2, snk_idx + 0.2, &p[6], &p[7]);
					_rel_to_abs(patch, src_idx + 0.2, snk_idx + 0.8, &p[4], &p[5]);
					_rel_to_abs(patch, src_idx + 0.8, snk_idx + 0.8, &p[2], &p[3]);
					_rel_to_abs(patch, src_idx + 0.8, snk_idx + 0.2, &p[0], &p[1]);

					nk_fill_polygon(canvas, p, 4, bright);
				}
				// EDGE
				else if(conn->enm & (VERTICAL_EDGE | HORIZONTAL_EDGE) )
				{
					_rel_to_abs(patch, src_idx + 0.38, snk_idx + 0.38, &p[6], &p[7]);
					_rel_to_abs(patch, src_idx + 0.38, snk_idx + 0.62, &p[4], &p[5]);
					_rel_to_abs(patch, src_idx + 0.62, snk_idx + 0.62, &p[2], &p[3]);
					_rel_to_abs(patch, src_idx + 0.62, snk_idx + 0.38, &p[0], &p[1]);

					nk_fill_polygon(canvas, p, 4, style->text.color);
				}
			}
		}

		// draw HORIZONTAL_LINES
		float yl, xl;
		for(int src_idx = 0; src_idx <= patch->src_n; src_idx++)
		{
			const int snk_idx = patch->snk_n;

			float p [6];
			_rel_to_abs(patch, src_idx, 0,       &p[0], &p[1]);
			_rel_to_abs(patch, src_idx, snk_idx, &p[2], &p[3]);
			p[4] = bounds.x; //-1.f;
			p[5] = p[3];

			float q [6];
			_rel_to_abs(patch, src_idx - 0.2, snk_idx, &q[0], &q[1]);
			_rel_to_abs(patch, src_idx - 0.8, snk_idx, &q[2], &q[3]);
			q[4] = q[2];
			q[5] = q[1];

			if(src_idx > 0)
			{
				const int c = src_idx - 1;
				const bool active = (src_ptr == c)
					|| ( (src_ptr == -1) && (snk_ptr != -1) && patch->connections[c][snk_ptr].state );

				nk_patcher_port_t *src_port = &patch->srcs[c];
				struct nk_rect label = nk_rect(p[4], yl, xl-p[4], p[3]-yl);
				const char *name = src_port->label;
				const size_t len = name ? nk_strlen(name) : 0;
				const struct nk_text text = {
					.padding.x = 2,
					.padding.y = 0,
					.background = style->window.background,
					.text = active ? src_port->color : style->text.color
				};

				if(active)
					nk_fill_rect(canvas, label, 0.f, style->button.active.data.color);
				nk_fill_polygon(canvas, q, 3, src_port->color);

				const struct nk_rect old_clip = canvas->clip;
				nk_push_scissor(canvas, label);
				nk_widget_text(canvas, label, name, len, &text,
					NK_TEXT_ALIGN_RIGHT | NK_TEXT_ALIGN_MIDDLE, style->font);
				nk_push_scissor(canvas, old_clip);
			}

			nk_stroke_polyline(canvas, p, 3, 2.f, style->window.border_color);
			xl = p[2];
			yl = p[3];
		}

		// draw VERTICAL_LINES
		for(int snk_idx = 0; snk_idx <= patch->snk_n; snk_idx++)
		{
			const int src_idx = patch->src_n;

			float p [6];
			_rel_to_abs(patch, 0,       snk_idx, &p[0], &p[1]);
			_rel_to_abs(patch, src_idx, snk_idx, &p[2], &p[3]);
			p[4] = bounds.x + bounds.w; //1.f;
			p[5] = p[3];

			float q[6];
			_rel_to_abs(patch, src_idx, snk_idx - 0.2, &q[0], &q[1]);
			_rel_to_abs(patch, src_idx, snk_idx - 0.8, &q[2], &q[3]);
			q[4] = q[2];
			q[5] = q[1];

			if(snk_idx > 0)
			{
				const int r = snk_idx - 1;
				const bool active = (snk_ptr == r)
					|| ( (src_ptr != -1) && (snk_ptr == -1) && patch->connections[src_ptr][r].state );

				nk_patcher_port_t *snk_port = &patch->snks[r];
				struct nk_rect label = nk_rect(xl, yl, p[4]-xl, p[3]-yl);
				const char *name = snk_port->label;
				const size_t len = name ? nk_strlen(name) : 0;
				const struct nk_text text = {
					.padding.x = 2,
					.padding.y = 0,
					.background = style->window.background,
					.text = active ? snk_port->color : style->text.color
				};

				if(active)
					nk_fill_rect(canvas, label, 0.f, style->button.active.data.color);
				nk_fill_polygon(canvas, q, 3, snk_port->color);

				const struct nk_rect old_clip = canvas->clip;
				nk_push_scissor(canvas, label);
				nk_widget_text(canvas, label, name, len, &text,
					NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE, style->font);
				nk_push_scissor(canvas, old_clip);
			}

			nk_stroke_polyline(canvas, p, 3, 2.f, style->window.border_color);
			xl = p[2];
			yl = p[3];
		}

		// draw HOVER
		if( (src_ptr != -1) && (snk_ptr != -1) )
		{
			float p [8];

			_rel_to_abs(patch, src_ptr + 0, snk_ptr + 0, &p[0], &p[1]);
			_rel_to_abs(patch, src_ptr + 0, snk_ptr + 1, &p[2], &p[3]);
			_rel_to_abs(patch, src_ptr + 1, snk_ptr + 1, &p[4], &p[5]);
			_rel_to_abs(patch, src_ptr + 1, snk_ptr + 0, &p[6], &p[7]);

			nk_stroke_polygon(canvas, p, 4, 2.f, bright);
		}
	}
}

static void
nk_patcher_fill(nk_patcher_t *patch, nk_patcher_fill_t fill, void *data)
{
	if(!fill)
		return;

	for(int src_idx=0; src_idx<patch->src_n; src_idx++)
	{
		nk_patcher_port_t *src_port = &patch->srcs[src_idx];

		for(int snk_idx=0; snk_idx<patch->snk_n; snk_idx++)
		{
			nk_patcher_port_t *snk_port = &patch->snks[snk_idx];
			nk_patcher_connection_t *conn = &patch->connections[src_idx][snk_idx];

			fill(data, src_port->id, snk_port->id, &conn->state, &conn->type);
		}
	}
}

#endif // NK_PATCHER_IMPLEMENTATION
