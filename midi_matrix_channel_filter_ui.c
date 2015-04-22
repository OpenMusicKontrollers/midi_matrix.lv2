/*
 * Copyright (c) 2015 Hanspeter Portner (dev@open-music-kontrollers.ch)
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

#include <midi_matrix.h>

#include <lv2_eo_ui.h>

#define BORDER_SIZE 12
#define TILE_SIZE 20

typedef struct _UI UI;

struct _UI {
	eo_ui_t eoui;

	LV2UI_Write_Function write_function;
	LV2UI_Controller controller;

	char theme_path [1024];

	Evas_Object *grid;
	Evas_Object *tiles [0x11][0x11];
	
	uint16_t mask [0x10];
};

// Center Tile Callbacks
static void
_tile_in(void *data, Evas_Object *edj, const char *emission, const char *source)
{
	UI *ui = data;
	int i, j;
	
	int x, y;
	elm_table_pack_get(edj, &x, &y, NULL, NULL);

	elm_layout_signal_emit(ui->tiles[x][y], "edge", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
	for(i=0x10; i>x; i--)
		elm_layout_signal_emit(ui->tiles[i][y], "horizontal", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
	for(j=0x10; j>y; j--)
		elm_layout_signal_emit(ui->tiles[x][j], "vertical", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
}

static void
_tile_out(void *data, Evas_Object *edj, const char *emission, const char *source)
{
	UI *ui = data;
	int i, j;
	
	int x, y;
	elm_table_pack_get(edj, &x, &y, NULL, NULL);

	elm_layout_signal_emit(ui->tiles[x][y], "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
	for(i=0x10; i>x; i--)
		elm_layout_signal_emit(ui->tiles[i][y], "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
	for(j=0x10; j>y; j--)
		elm_layout_signal_emit(ui->tiles[x][j], "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
}

static uint16_t
_tile_refresh(UI *ui, int i, uint16_t src, uint16_t dst)
{
	uint16_t change = dst ^ src;

	if(change)
	{
		uint16_t j;
		uint16_t mask;
		for(j=0, mask=1; j<0x10; j++, mask=mask<<1)
			if(change & mask)
			{
				if(dst & mask)
					elm_layout_signal_emit(ui->tiles[i][j], "on", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
				else
					elm_layout_signal_emit(ui->tiles[i][j], "off", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
			}
	}

	return dst;
}

static void
_tile_changed(void *data, Evas_Object *edj, const char *emission, const char *source)
{
	UI *ui = data;

	int i, j;
	elm_table_pack_get(edj, &i, &j, NULL, NULL);

	uint16_t *src = &ui->mask[i];
	uint16_t mask = 1 << j;
	if(*src & mask) // on -> off
		*src = _tile_refresh(ui, i, *src, *src & ~mask);
	else // off -> on
		*src = _tile_refresh(ui, i, *src, *src | mask);

	float control = *src;
	ui->write_function(ui->controller, i, sizeof(float), 0, &control);
}

// Input Channel Callbacks
static void
_horizontal_in(void *data, Evas_Object *edj, const char *emission, const char *source)
{
	UI *ui = data;
	int i, j;
	int x, y;

	elm_table_pack_get(edj, &x, &y, NULL, NULL);
	elm_layout_signal_emit(ui->tiles[x][y], "edge", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);

	int vert = 0;
	uint16_t mask = ui->mask[x];
	uint16_t dst;
	for(i=0, dst=1; i<0x10; i++, dst=dst<<1)
		if(mask & dst)
		{
			elm_layout_signal_emit(ui->tiles[x][i], "highlight,on", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
			if(!vert)
			{
				elm_layout_signal_emit(ui->tiles[x][i], "edge", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
				for(j=0x10; j>i; j--)
					elm_layout_signal_emit(ui->tiles[x][j], "vertical", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
				vert = 1;
			}
			else // vert
				elm_layout_signal_emit(ui->tiles[x][i], "edge,vertical", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
			for(j=0x10; j>x; j--)
				elm_layout_signal_emit(ui->tiles[j][i], "horizontal", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
		}
}

static void
_horizontal_out(void *data, Evas_Object *edj, const char *emission, const char *source)
{
	UI *ui = data;
	int i, j;
	int x, y;

	elm_table_pack_get(edj, &x, &y, NULL, NULL);
	elm_layout_signal_emit(ui->tiles[x][y], "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);

	int vert = 0;
	uint16_t mask = ui->mask[x];
	uint16_t dst;
	for(i=0, dst=1; i<0x10; i++, dst=dst<<1)
		if(mask & dst)
		{
			elm_layout_signal_emit(ui->tiles[x][i], "highlight,off", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
			if(!vert)
			{
				elm_layout_signal_emit(ui->tiles[x][i], "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
				for(j=0x10; j>i; j--)
					elm_layout_signal_emit(ui->tiles[x][j], "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
				vert = 1;
			}
			else // vert
				elm_layout_signal_emit(ui->tiles[x][i], "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
			for(j=0x10; j>x; j--)
				elm_layout_signal_emit(ui->tiles[j][i], "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
		}
}

static void
_horizontal_changed(void *data, Evas_Object *edj, const char *emission, const char *source)
{
	UI *ui = data;
	
	_horizontal_out(data, edj, emission, source);

	int i, j;
	elm_table_pack_get(edj, &i, &j, NULL, NULL);

	uint16_t *src = &ui->mask[i];
	*src = _tile_refresh(ui, i, *src, *src ? 0x0000 : 0xffff);

	float control = *src;
	ui->write_function(ui->controller, i, sizeof(float), 0, &control);
	
	_horizontal_in(data, edj, emission, source);
}

// Output Channel Callbacks
static void
_vertical_in(void *data, Evas_Object *edj, const char *emission, const char *source)
{
	UI *ui = data;
	int i, j;
	int x, y;

	elm_table_pack_get(edj, &x, &y, NULL, NULL);
	elm_layout_signal_emit(ui->tiles[x][y], "edge", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);

	int horz = 0;
	uint16_t mask = 1<<y;
	for(i=0; i<0x10; i++)
		if(mask & ui->mask[i])
		{
			elm_layout_signal_emit(ui->tiles[i][y], "highlight,on", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
			if(!horz)
			{
				elm_layout_signal_emit(ui->tiles[i][y], "edge", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
				for(j=0x10; j>i; j--)
					elm_layout_signal_emit(ui->tiles[j][y], "horizontal", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);

				horz = 1;
			}
			else // horz
				elm_layout_signal_emit(ui->tiles[i][y], "edge,horizontal", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
			for(j=0x10; j>y; j--)
				elm_layout_signal_emit(ui->tiles[i][j], "vertical", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
		}
}

static void
_vertical_out(void *data, Evas_Object *edj, const char *emission, const char *source)
{
	UI *ui = data;
	int i, j;
	int x, y;

	elm_table_pack_get(edj, &x, &y, NULL, NULL);
	elm_layout_signal_emit(ui->tiles[x][y], "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);

	int horz = 0;
	uint16_t mask = 1<<y;
	for(i=0; i<0x10; i++)
		if(mask & ui->mask[i])
		{
			elm_layout_signal_emit(ui->tiles[i][y], "highlight,off", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
			if(!horz)
			{
				elm_layout_signal_emit(ui->tiles[i][y], "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
				for(j=0x10; j>i; j--)
					elm_layout_signal_emit(ui->tiles[j][y], "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);

				horz = 1;
			}
			else // horz
				elm_layout_signal_emit(ui->tiles[i][y], "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
			for(j=0x10; j>y; j--)
				elm_layout_signal_emit(ui->tiles[i][j], "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
		}
}

static void
_vertical_changed(void *data, Evas_Object *edj, const char *emission, const char *source)
{
	UI *ui = data;
	
	_vertical_out(data, edj, emission, source);

	int i, j;
	elm_table_pack_get(edj, &i, &j, NULL, NULL);

	uint16_t mask = 1 << j;
	int clear = 0;

	int c;
	for(c=0; c<0x10; c++)
	{
		uint16_t *src = &ui->mask[c];
		if(*src & mask)
		{
			clear = 1;
			break;
		}
	}

	for(c=0; c<0x10; c++)
	{
		uint16_t *src = &ui->mask[c];
		*src = _tile_refresh(ui, c, *src, clear ? *src & ~mask : *src | mask);

		float control = *src;
		ui->write_function(ui->controller, i, sizeof(float), 0, &control);
	}

	_vertical_in(data, edj, emission, source);
}

// Shortcut Button Callbacks
static void
_DAC_in(void *data, Evas_Object *edj, const char *emission, const char *source)
{
	elm_layout_signal_emit(edj, "edge", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
}

static void
_DAC_out(void *data, Evas_Object *edj, const char *emission, const char *source)
{
	elm_layout_signal_emit(edj, "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
}

static void
_def_changed(void *data, Evas_Object *edj, const char *emission, const char *source)
{
	UI *ui = data;

	int i;
	for(i=0; i<0x10; i++)
	{
		uint16_t *src = &ui->mask[i];
		uint16_t dst = 1 << i;
		*src = _tile_refresh(ui, i, *src, dst);
		float control = *src;
		ui->write_function(ui->controller, i, sizeof(float), 0, &control);
	}
}

static void
_all_changed(void *data, Evas_Object *edj, const char *emission, const char *source)
{
	UI *ui = data;

	int i;
	for(i=0; i<0x10; i++)
	{
		uint16_t *src = &ui->mask[i];
		uint16_t dst = 0xffff;
		*src = _tile_refresh(ui, i, *src, dst);
		float control = *src;
		ui->write_function(ui->controller, i, sizeof(float), 0, &control);
	}
}

static void
_clear_changed(void *data, Evas_Object *edj, const char *emission, const char *source)
{
	UI *ui = data;

	int i;
	for(i=0; i<0x10; i++)
	{
		uint16_t *src = &ui->mask[i];
		uint16_t dst = 0x0;
		*src = _tile_refresh(ui, i, *src, dst);
		float control = *src;
		ui->write_function(ui->controller, i, sizeof(float), 0, &control);
	}
}

static Evas_Object *
_content_get(eo_ui_t *eoui)
{
	UI *ui = (void *)eoui - offsetof(UI, eoui);

	ui->grid = elm_table_add(eoui->win);
	elm_table_homogeneous_set(ui->grid, EINA_TRUE);
	elm_table_padding_set(ui->grid, 0, 0);
	evas_object_size_hint_aspect_set(ui->grid, EVAS_ASPECT_CONTROL_BOTH, 1, 1);

	for(int i=0; i<0x10; i++)
	{
		for(int j=0; j<0x10; j++)
		{
			Evas_Object *tile = elm_layout_add(ui->grid);
			elm_layout_file_set(tile, ui->theme_path,
				MIDI_MATRIX_CHANNEL_FILTER_UI_URI"/tile");
			elm_layout_signal_callback_add(tile, "in",
				MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _tile_in, ui);
			elm_layout_signal_callback_add(tile, "out",
				MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _tile_out, ui);
			elm_layout_signal_callback_add(tile, "changed",
				MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _tile_changed, ui);
			evas_object_size_hint_weight_set(tile, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_size_hint_align_set(tile, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_show(tile);
			elm_table_pack(ui->grid, tile, i, j, 1, 1);
			ui->tiles[i][j] = tile;
		}

		char str [3];
		Evas_Object *label;

		sprintf(str, "%02i", i+1);
		label = elm_layout_add(ui->grid);
		elm_layout_file_set(label, ui->theme_path,
			MIDI_MATRIX_CHANNEL_FILTER_UI_URI"/label");
		elm_object_part_text_set(label, "default", str);
		elm_layout_signal_callback_add(label, "in",
			MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _horizontal_in, ui);
		elm_layout_signal_callback_add(label, "out",
			MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _horizontal_out, ui);
		elm_layout_signal_callback_add(label, "changed",
			MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _horizontal_changed, ui);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_show(label);
		elm_table_pack(ui->grid, label, i, 0x10, 1, 1);
		ui->tiles[i][0x10] = label;

		sprintf(str, "%02i", i+1);
		label = elm_layout_add(ui->grid);
		elm_layout_file_set(label, ui->theme_path,
			MIDI_MATRIX_CHANNEL_FILTER_UI_URI"/label");
		elm_object_part_text_set(label, "default", str);
		elm_layout_signal_callback_add(label, "in",
			MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _vertical_in, ui);
		elm_layout_signal_callback_add(label, "out",
			MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _vertical_out, ui);
		elm_layout_signal_callback_add(label, "changed",
			MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _vertical_changed, ui);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_show(label);
		elm_table_pack(ui->grid, label, 0x10, i, 1, 1);
		ui->tiles[0x10][i] = label;
	}

	Evas_Object *def = elm_layout_add(ui->grid);
	elm_layout_file_set(def, ui->theme_path,
		MIDI_MATRIX_CHANNEL_FILTER_UI_URI"/label");
	elm_object_part_text_set(def, "default", "D");
	elm_layout_signal_callback_add(def, "in",
		MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _DAC_in, ui);
	elm_layout_signal_callback_add(def, "out",
		MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _DAC_out, ui);
	elm_layout_signal_callback_add(def, "changed",
		MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _def_changed, ui);
	evas_object_size_hint_weight_set(def, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(def, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(def);
	elm_table_pack(ui->grid, def, 0x11, 0x10, 1, 1);

	Evas_Object *all = elm_layout_add(ui->grid);
	elm_layout_file_set(all, ui->theme_path,
		MIDI_MATRIX_CHANNEL_FILTER_UI_URI"/label");
	elm_object_part_text_set(all, "default", "A");
	elm_layout_signal_callback_add(all, "in",
		MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _DAC_in, ui);
	elm_layout_signal_callback_add(all, "out",
		MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _DAC_out, ui);
	elm_layout_signal_callback_add(all, "changed",
		MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _all_changed, ui);
	evas_object_size_hint_weight_set(all, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(all, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(all);
	elm_table_pack(ui->grid, all, 0x10, 0x11, 1, 1);

	Evas_Object *clear = elm_layout_add(ui->grid);
	elm_layout_file_set(clear, ui->theme_path,
		MIDI_MATRIX_CHANNEL_FILTER_UI_URI"/label");
	elm_object_part_text_set(clear, "default", "C");
	elm_layout_signal_callback_add(clear, "in",
		MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _DAC_in, ui);
	elm_layout_signal_callback_add(clear, "out",
		MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _DAC_out, ui);
	elm_layout_signal_callback_add(clear, "changed",
		MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _clear_changed, ui);
	evas_object_size_hint_weight_set(clear, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(clear, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(clear);
	elm_table_pack(ui->grid, clear, 0x11, 0x11, 1, 1);

	Evas_Object *input_label = elm_layout_add(ui->grid);
	elm_layout_file_set(input_label, ui->theme_path,
		MIDI_MATRIX_CHANNEL_FILTER_UI_URI"/label/inputs");
	elm_object_part_text_set(input_label, "default", "Input Channels");
	evas_object_size_hint_weight_set(input_label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(input_label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(input_label);
	elm_table_pack(ui->grid, input_label, 0, 0x11, 0x10, 1);

	Evas_Object *output_label = elm_layout_add(ui->grid);
	elm_layout_file_set(output_label, ui->theme_path,
		MIDI_MATRIX_CHANNEL_FILTER_UI_URI"/label/outputs");
	elm_object_part_text_set(output_label, "default", "Output Channels");
	evas_object_size_hint_weight_set(output_label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(output_label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(output_label);
	elm_table_pack(ui->grid, output_label, 0x11, 0, 1, 0x10);

	return ui->grid;
}

static LV2UI_Handle
instantiate(const LV2UI_Descriptor *descriptor, const char *plugin_uri, const char *bundle_path, LV2UI_Write_Function write_function, LV2UI_Controller controller, LV2UI_Widget *widget, const LV2_Feature *const *features)
{
	if(strcmp(plugin_uri, MIDI_MATRIX_CHANNEL_FILTER_URI))
		return NULL;

	eo_ui_driver_t driver;
	if(descriptor == &channel_filter_eo)
		driver = EO_UI_DRIVER_EO;
	else if(descriptor == &channel_filter_ui)
		driver = EO_UI_DRIVER_UI;
	else if(descriptor == &channel_filter_x11)
		driver = EO_UI_DRIVER_X11;
	else if(descriptor == &channel_filter_kx)
		driver = EO_UI_DRIVER_KX;
	else
		return NULL;

	UI *ui = calloc(1, sizeof(UI));
	if(!ui)
		return NULL;

	eo_ui_t *eoui = &ui->eoui;
	eoui->driver = driver;
	eoui->content_get = _content_get;
	eoui->w = 400,
	eoui->h = 400;

	ui->write_function = write_function;
	ui->controller = controller;

	int i, j;
	for(i=0; features[i]; i++)
	{
		// nothing
  }
	
	sprintf(ui->theme_path, "%s/midi_matrix.edj", bundle_path);

	if(eoui_instantiate(eoui, descriptor, plugin_uri, bundle_path, write_function,
		controller, widget, features))
	{
		free(ui);
		return NULL;
	}
	
	return ui;
}

static void
cleanup(LV2UI_Handle handle)
{
	UI *ui = handle;
	
	eoui_cleanup(&ui->eoui);
	
	if(ui)
		free(ui);
}

static void
port_event(LV2UI_Handle handle, uint32_t i, uint32_t buffer_size, uint32_t format, const void *buffer)
{
	UI *ui = handle;

	if(i < 0x10)
	{
		uint16_t *src = &ui->mask[i];
		uint16_t dst = (uint16_t)*(const float *)buffer;

		*src = _tile_refresh(ui, i, *src, dst);
	}
}

static const void *
extension_data(const char *uri)
{
	if(!strcmp(uri, LV2_UI__idleInterface))
		return &idle_ext;
	else if(!strcmp(uri, LV2_UI__showInterface))
		return &show_ext;
		
	return NULL;
}

const LV2UI_Descriptor channel_filter_eo = {
	.URI						= MIDI_MATRIX_CHANNEL_FILTER_EO_URI,
	.instantiate		= instantiate,
	.cleanup				= cleanup,
	.port_event			= port_event,
	.extension_data	= eoui_eo_extension_data
};

const LV2UI_Descriptor channel_filter_ui = {
	.URI						= MIDI_MATRIX_CHANNEL_FILTER_UI_URI,
	.instantiate		= instantiate,
	.cleanup				= cleanup,
	.port_event			= port_event,
	.extension_data	= eoui_ui_extension_data
};

const LV2UI_Descriptor channel_filter_x11 = {
	.URI						= MIDI_MATRIX_CHANNEL_FILTER_X11_URI,
	.instantiate		= instantiate,
	.cleanup				= cleanup,
	.port_event			= port_event,
	.extension_data	= eoui_x11_extension_data
};

const LV2UI_Descriptor channel_filter_kx = {
	.URI						= MIDI_MATRIX_CHANNEL_FILTER_KX_URI,
	.instantiate		= instantiate,
	.cleanup				= cleanup,
	.port_event			= port_event,
	.extension_data	= eoui_kx_extension_data
};
