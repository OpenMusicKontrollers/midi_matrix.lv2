/*
 * Copyright (c) 2016 Hanspeter Portner (dev@open-music-kontrollers.ch)
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the Artistic License 2.0 as published by
 * The Perl Foundation.
 *
 * This source is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * Artistic License 2.0 for more details.
 *
 * You should have received a copy of the Artistic License 2.0
 * along the source as a COPYING file. If not, obtain it from
 * http://www.perlfoundation.org/artistic_license_2_0.
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <inttypes.h>

#include <midi_matrix.h>

#define NK_PUGL_IMPLEMENTATION
#include "nk_pugl/nk_pugl.h"

#define NK_PATCHER_IMPLEMENTATION
#include <nk_patcher.h>

typedef struct _plughandle_t plughandle_t;

struct _plughandle_t {
	LV2_URID_Map *map;
	LV2_Atom_Forge forge;

	LV2_URID ui_floatProtocol;

	nk_pugl_window_t win;

	LV2UI_Controller *controller;
	LV2UI_Write_Function writer;

	float dy;

	uint16_t mask [0x10];

	nk_patcher_t patch;
};

static void
_change(void *data, uintptr_t src_id, uintptr_t snk_id, bool state)
{
	plughandle_t *handle = data;

	uint16_t *dst = &handle->mask[src_id];
	const uint16_t bits = 1 << snk_id;

	if(state)
		*dst |= bits;
	else
		*dst &= ~bits;

	const float val = *dst;
	handle->writer(handle->controller, src_id, sizeof(float), 0, &val);

	nk_patcher_connected_set(&handle->patch, src_id, snk_id, state, NK_PATCHER_TYPE_DIRECT);
	nk_pugl_post_redisplay(&handle->win);
}

static void
_expose(struct nk_context *ctx, struct nk_rect wbounds, void *data)
{
	plughandle_t *handle = data;

	const float dy = handle->dy;
	struct nk_style *style = &ctx->style;
	const struct nk_vec2 group_padding = nk_panel_get_padding(style, NK_PANEL_WINDOW);

	if(nk_begin(ctx, "ChannelFilter", wbounds, NK_WINDOW_NO_SCROLLBAR))
	{
		nk_window_set_bounds(ctx, wbounds);
		struct nk_panel *center = nk_window_get_panel(ctx);

		struct nk_rect bounds = center->bounds;
		bounds.x += group_padding.x;
		bounds.y += group_padding.y;
		bounds.w -= 2*group_padding.y;
		bounds.h -= 2*group_padding.y;

		nk_layout_row_dynamic(ctx, wbounds.h, 1);
		nk_patcher_render(&handle->patch, ctx, wbounds, _change, handle);
	}
	nk_end(ctx);
}

static LV2UI_Handle
instantiate(const LV2UI_Descriptor *descriptor, const char *plugin_uri,
	const char *bundle_path, LV2UI_Write_Function write_function,
	LV2UI_Controller controller, LV2UI_Widget *widget,
	const LV2_Feature *const *features)
{
	plughandle_t *handle = calloc(1, sizeof(plughandle_t));
	if(!handle)
		return NULL;

	void *parent = NULL;
	LV2UI_Resize *host_resize = NULL;
	for(int i=0; features[i]; i++)
	{
		if(!strcmp(features[i]->URI, LV2_UI__parent))
			parent = features[i]->data;
		else if(!strcmp(features[i]->URI, LV2_UI__resize))
			host_resize = features[i]->data;
		else if(!strcmp(features[i]->URI, LV2_URID__map))
			handle->map = features[i]->data;
	}

	if(!parent)
	{
		fprintf(stderr,
			"%s: Host does not support ui:parent\n", descriptor->URI);
		free(handle);
		return NULL;
	}
	if(!handle->map)
	{
		fprintf(stderr,
			"%s: Host does not support urid:map\n", descriptor->URI);
		free(handle);
		return NULL;
	}

	lv2_atom_forge_init(&handle->forge, handle->map);

	handle->ui_floatProtocol = handle->map->map(handle->map->handle, LV2_UI__floatProtocol);

	handle->controller = controller;
	handle->writer = write_function;

	const char *NK_SCALE = getenv("NK_SCALE");
	const float scale = NK_SCALE ? atof(NK_SCALE) : 1.f;
	handle->dy = 20.f * scale;

	nk_pugl_config_t *cfg = &handle->win.cfg;
	cfg->width = 640* scale;
	cfg->height = 640* scale;
	cfg->resizable = true;
	cfg->ignore = false;
	cfg->class = "channel_filter";
	cfg->title = "Channel Filter";
	cfg->parent = (intptr_t)parent;
	cfg->data = handle;
	cfg->expose = _expose;

	char *path;
	if(asprintf(&path, "%sCousine-Regular.ttf", bundle_path) == -1)
		path = NULL;

	cfg->font.face = path;
	cfg->font.size = 13 * scale;
	
	*(intptr_t *)widget = nk_pugl_init(&handle->win);
	nk_pugl_show(&handle->win);

	if(path)
		free(path);

	if(host_resize)
		host_resize->ui_resize(host_resize->handle, cfg->width, cfg->height);

	nk_patcher_init(&handle->patch, 0.8f);
	nk_patcher_reinit(&handle->patch, 0x10, 0x10);

	for(int i = 0; i < 0x10; i++)
	{
		nk_patcher_src_id_set(&handle->patch, i, i);
		nk_patcher_snk_id_set(&handle->patch, i, i);

		char tmp [32];
		snprintf(tmp, 32, "IN-%02i", i + 1);
		nk_patcher_src_label_set(&handle->patch, i, tmp);
		snprintf(tmp, 32, "OUT-%02i", i + 1);
		nk_patcher_snk_label_set(&handle->patch, i, tmp);

		nk_patcher_src_color_set(&handle->patch, i, nk_rgb(0xbb, 0x00, 0x00));
		nk_patcher_snk_color_set(&handle->patch, i, nk_rgb(0xbb, 0x00, 0x00));
	}

	return handle;
}

static void
cleanup(LV2UI_Handle instance)
{
	plughandle_t *handle = instance;

	nk_patcher_deinit(&handle->patch);

	nk_pugl_hide(&handle->win);
	nk_pugl_shutdown(&handle->win);

	free(handle);
}

static void
port_event(LV2UI_Handle instance, uint32_t index, uint32_t size,
	uint32_t protocol, const void *buf)
{
	plughandle_t *handle = instance;

	if( (protocol == 0) || (protocol == handle->ui_floatProtocol) )
	{
		if(index < 0x10)
		{
			uint16_t *dst = &handle->mask[index];
			*dst = *(const float *)buf;

			for(int snk_id = 0; snk_id < 0x10; snk_id++)
			{
				const uint16_t bits = 1 << snk_id;
				const bool sel = (*dst & bits) == bits;

				nk_patcher_connected_set(&handle->patch, index, snk_id, sel, NK_PATCHER_TYPE_DIRECT);
			}

			nk_pugl_post_redisplay(&handle->win);
		}
	}
}

static int
_idle(LV2UI_Handle instance)
{
	plughandle_t *handle = instance;

	return nk_pugl_process_events(&handle->win);
}

static const LV2UI_Idle_Interface idle_ext = {
	.idle = _idle
};

static const void *
extension_data(const char *uri)
{
	if(!strcmp(uri, LV2_UI__idleInterface))
		return &idle_ext;
		
	return NULL;
}

const LV2UI_Descriptor channel_filter_nk = {
	.URI            = MIDI_MATRIX_CHANNEL_FILTER_NK_URI,
	.instantiate    = instantiate,
	.cleanup        = cleanup,
	.port_event     = port_event,
	.extension_data = extension_data
};
