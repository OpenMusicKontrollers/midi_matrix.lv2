/*
 * Copyright (c) 2014 Hanspeter Portner (dev@open-music-kontrollers.ch)
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 *     1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 * 
 *     2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 * 
 *     3. This notice may not be removed or altered from any source
 *     distribution.
 */

#include <midi_matrix.h>

#include <Eina.h>
#include <Ecore.h>
#include <Evas.h>
#include <Ecore_Evas.h>
#include <Edje.h>

#define BORDER_SIZE 12
#define TILE_SIZE 20

typedef struct _UI UI;

struct _UI {
	LV2UI_Write_Function write_function;
	LV2UI_Controller controller;

	int w, h;
	Ecore_Evas *ee;
	Evas *e;
	Evas_Object *theme;
	Evas_Object *grid;
	Evas_Object *tiles [0x11][0x11];
	
	uint16_t mask [0x10];
};

// Idle interface
static int
idle_cb(LV2UI_Handle handle)
{
	UI *ui = handle;

	if(!ui)
		return -1;

	ecore_main_loop_iterate();
	
	return 0;
}

static const LV2UI_Idle_Interface idle_ext = {
	.idle = idle_cb
};

// Show Interface
static int
_show_cb(LV2UI_Handle handle)
{
	UI *ui = handle;

	if(!ui)
		return -1;

	if(ui->ee)
		ecore_evas_show(ui->ee);

	return 0;
}

static int
_hide_cb(LV2UI_Handle handle)
{
	UI *ui = handle;

	if(!ui)
		return -1;

	if(ui->ee)
		ecore_evas_hide(ui->ee);

	return 0;
}

static const LV2UI_Show_Interface show_ext = {
	.show = _show_cb,
	.hide = _hide_cb
};

// Resize Interface
static int
resize_cb(LV2UI_Feature_Handle handle, int w, int h)
{
	UI *ui = handle;

	if(!ui)
		return -1;

	ui->w = w;
	ui->h = h;

	if(ui->ee)
		ecore_evas_resize(ui->ee, ui->w, ui->h);
	evas_object_resize(ui->theme, ui->w, ui->h);
  
  return 0;
}

// Center Tile Callbacks
static void
_tile_in(void *data, Evas_Object *edj, const char *emission, const char *source)
{
	UI *ui = data;
	int i, j;
	
	unsigned short x, y;
	evas_object_table_pack_get(ui->grid, edj, &x, &y, NULL, NULL);

	edje_object_signal_emit(ui->tiles[x][y], "edge", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
	for(i=0x10; i>x; i--)
		edje_object_signal_emit(ui->tiles[i][y], "horizontal", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
	for(j=0x10; j>y; j--)
		edje_object_signal_emit(ui->tiles[x][j], "vertical", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
}

static void
_tile_out(void *data, Evas_Object *edj, const char *emission, const char *source)
{
	UI *ui = data;
	int i, j;
	
	unsigned short x, y;
	evas_object_table_pack_get(ui->grid, edj, &x, &y, NULL, NULL);

	edje_object_signal_emit(ui->tiles[x][y], "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
	for(i=0x10; i>x; i--)
		edje_object_signal_emit(ui->tiles[i][y], "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
	for(j=0x10; j>y; j--)
		edje_object_signal_emit(ui->tiles[x][j], "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
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
					edje_object_signal_emit(ui->tiles[i][j], "on", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
				else
					edje_object_signal_emit(ui->tiles[i][j], "off", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
			}
	}

	return dst;
}

static void
_tile_changed(void *data, Evas_Object *edj, const char *emission, const char *source)
{
	UI *ui = data;

	unsigned short i, j;
	evas_object_table_pack_get(ui->grid, edj, &i, &j, NULL, NULL);

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
	unsigned short x, y;

	evas_object_table_pack_get(ui->grid, edj, &x, &y, NULL, NULL);
	edje_object_signal_emit(ui->tiles[x][y], "edge", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);

	int vert = 0;
	uint16_t mask = ui->mask[x];
	uint16_t dst;
	for(i=0, dst=1; i<0x10; i++, dst=dst<<1)
		if(mask & dst)
		{
			edje_object_signal_emit(ui->tiles[x][i], "highlight,on", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
			if(!vert)
			{
				edje_object_signal_emit(ui->tiles[x][i], "edge", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
				for(j=0x10; j>i; j--)
					edje_object_signal_emit(ui->tiles[x][j], "vertical", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
				vert = 1;
			}
			else // vert
				edje_object_signal_emit(ui->tiles[x][i], "edge,vertical", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
			for(j=0x10; j>x; j--)
				edje_object_signal_emit(ui->tiles[j][i], "horizontal", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
		}
}

static void
_horizontal_out(void *data, Evas_Object *edj, const char *emission, const char *source)
{
	UI *ui = data;
	int i, j;
	unsigned short x, y;

	evas_object_table_pack_get(ui->grid, edj, &x, &y, NULL, NULL);
	edje_object_signal_emit(ui->tiles[x][y], "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);

	int vert = 0;
	uint16_t mask = ui->mask[x];
	uint16_t dst;
	for(i=0, dst=1; i<0x10; i++, dst=dst<<1)
		if(mask & dst)
		{
			edje_object_signal_emit(ui->tiles[x][i], "highlight,off", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
			if(!vert)
			{
				edje_object_signal_emit(ui->tiles[x][i], "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
				for(j=0x10; j>i; j--)
					edje_object_signal_emit(ui->tiles[x][j], "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
				vert = 1;
			}
			else // vert
				edje_object_signal_emit(ui->tiles[x][i], "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
			for(j=0x10; j>x; j--)
				edje_object_signal_emit(ui->tiles[j][i], "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
		}
}

static void
_horizontal_changed(void *data, Evas_Object *edj, const char *emission, const char *source)
{
	UI *ui = data;
	
	_horizontal_out(data, edj, emission, source);

	unsigned short i, j;
	evas_object_table_pack_get(ui->grid, edj, &i, &j, NULL, NULL);

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
	unsigned short x, y;

	evas_object_table_pack_get(ui->grid, edj, &x, &y, NULL, NULL);
	edje_object_signal_emit(ui->tiles[x][y], "edge", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);

	int horz = 0;
	uint16_t mask = 1<<y;
	for(i=0; i<0x10; i++)
		if(mask & ui->mask[i])
		{
			edje_object_signal_emit(ui->tiles[i][y], "highlight,on", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
			if(!horz)
			{
				edje_object_signal_emit(ui->tiles[i][y], "edge", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
				for(j=0x10; j>i; j--)
					edje_object_signal_emit(ui->tiles[j][y], "horizontal", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);

				horz = 1;
			}
			else // horz
				edje_object_signal_emit(ui->tiles[i][y], "edge,horizontal", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
			for(j=0x10; j>y; j--)
				edje_object_signal_emit(ui->tiles[i][j], "vertical", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
		}
}

static void
_vertical_out(void *data, Evas_Object *edj, const char *emission, const char *source)
{
	UI *ui = data;
	int i, j;
	unsigned short x, y;

	evas_object_table_pack_get(ui->grid, edj, &x, &y, NULL, NULL);
	edje_object_signal_emit(ui->tiles[x][y], "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);

	int horz = 0;
	uint16_t mask = 1<<y;
	for(i=0; i<0x10; i++)
		if(mask & ui->mask[i])
		{
			edje_object_signal_emit(ui->tiles[i][y], "highlight,off", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
			if(!horz)
			{
				edje_object_signal_emit(ui->tiles[i][y], "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
				for(j=0x10; j>i; j--)
					edje_object_signal_emit(ui->tiles[j][y], "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);

				horz = 1;
			}
			else // horz
				edje_object_signal_emit(ui->tiles[i][y], "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
			for(j=0x10; j>y; j--)
				edje_object_signal_emit(ui->tiles[i][j], "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
		}
}

static void
_vertical_changed(void *data, Evas_Object *edj, const char *emission, const char *source)
{
	UI *ui = data;
	
	_vertical_out(data, edj, emission, source);

	unsigned short i, j;
	evas_object_table_pack_get(ui->grid, edj, &i, &j, NULL, NULL);

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
	edje_object_signal_emit(edj, "edge", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
}

static void
_DAC_out(void *data, Evas_Object *edj, const char *emission, const char *source)
{
	edje_object_signal_emit(edj, "clear", MIDI_MATRIX_CHANNEL_FILTER_UI_URI);
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

static void
_delete(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	UI *ui = data;
	
	edje_object_part_unswallow(ui->theme, ui->grid);
	evas_object_table_clear(ui->grid, EINA_TRUE);
	evas_object_del(ui->grid);
}

static LV2UI_Handle
instantiate(const LV2UI_Descriptor *descriptor, const char *plugin_uri, const char *bundle_path, LV2UI_Write_Function write_function, LV2UI_Controller controller, LV2UI_Widget *widget, const LV2_Feature *const *features)
{
	eina_init();
	ecore_init();
	evas_init();
	ecore_evas_init();
	edje_init();

	//edje_frametime_set(0.04);

	if(strcmp(plugin_uri, MIDI_MATRIX_CHANNEL_FILTER_URI))
		return NULL;

	UI *ui = calloc(1, sizeof(UI));
	if(!ui)
		return NULL;

	ui->w = BORDER_SIZE*2 + TILE_SIZE*0x12;
	ui->h = BORDER_SIZE*2 + TILE_SIZE*0x12;
	ui->write_function = write_function;
	ui->controller = controller;

	void *parent = NULL;
	LV2UI_Resize *resize = NULL;
	
	int i, j;
	for(i=0; features[i]; i++)
	{
		if(!strcmp(features[i]->URI, LV2_UI__parent))
			parent = features[i]->data;
		else if (!strcmp(features[i]->URI, LV2_UI__resize))
			resize = (LV2UI_Resize *)features[i]->data;
  }

	if(descriptor == &lv2_midi_matrix_channel_filter_ui)
	{
		ui->ee = ecore_evas_gl_x11_new(NULL, (Ecore_X_Window)parent, 0, 0, ui->w, ui->h);
		if(!ui->ee)
			ui->ee = ecore_evas_software_x11_new(NULL, (Ecore_X_Window)parent, 0, 0, ui->w, ui->h);
		if(!ui->ee)
			printf("could not start evas\n");
		ui->e = ecore_evas_get(ui->ee);
		ecore_evas_show(ui->ee);
	}
	else if(descriptor == &lv2_midi_matrix_channel_filter_eo)
	{
		ui->ee = NULL;
		ui->e = evas_object_evas_get((Evas_Object *)parent);
	}

	if(resize)
    resize->ui_resize(resize->handle, ui->w, ui->h);

	char buf [512];
	sprintf(buf, "%s/midi_matrix.edj", bundle_path);
	
	ui->theme = edje_object_add(ui->e);
	edje_object_file_set(ui->theme, buf, MIDI_MATRIX_CHANNEL_FILTER_UI_URI"/theme");
	evas_object_event_callback_add(ui->theme, EVAS_CALLBACK_DEL, _delete, ui);
	evas_object_size_hint_weight_set(ui->theme, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ui->theme, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_aspect_set(ui->theme, EVAS_ASPECT_CONTROL_BOTH,
		ui->w, ui->h);
	evas_object_resize(ui->theme, ui->w, ui->h);
	evas_object_show(ui->theme);

	ui->grid = evas_object_table_add(ui->e);
	evas_object_table_homogeneous_set(ui->grid, EINA_TRUE);
	evas_object_resize(ui->grid, ui->w, ui->h);
	evas_object_show(ui->grid);
	edje_object_part_swallow(ui->theme, "matrix", ui->grid);

	for(i=0; i<0x10; i++)
	{
		for(j=0; j<0x10; j++)
		{
			Evas_Object *tile = edje_object_add(ui->e);
			edje_object_file_set(tile, buf, MIDI_MATRIX_CHANNEL_FILTER_UI_URI"/tile");
			edje_object_signal_callback_add(tile, "in",
				MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _tile_in, ui);
			edje_object_signal_callback_add(tile, "out",
				MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _tile_out, ui);
			edje_object_signal_callback_add(tile, "changed",
				MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _tile_changed, ui);
			evas_object_size_hint_weight_set(tile, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_size_hint_align_set(tile, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_show(tile);
			evas_object_table_pack(ui->grid, tile, i, j, 1, 1);
			ui->tiles[i][j] = tile;
		}

		char str [3];
		Evas_Object *label;

		sprintf(str, "%02i", i+1);
		label = edje_object_add(ui->e);
		edje_object_file_set(label, buf, MIDI_MATRIX_CHANNEL_FILTER_UI_URI"/label");
		edje_object_part_text_set(label, "default", str);
		edje_object_signal_callback_add(label, "in", MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _horizontal_in, ui);
		edje_object_signal_callback_add(label, "out", MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _horizontal_out, ui);
		edje_object_signal_callback_add(label, "changed", MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _horizontal_changed, ui);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_show(label);
		evas_object_table_pack(ui->grid, label, i, 0x10, 1, 1);
		ui->tiles[i][0x10] = label;

		sprintf(str, "%02i", i+1);
		label = edje_object_add(ui->e);
		edje_object_file_set(label, buf, MIDI_MATRIX_CHANNEL_FILTER_UI_URI"/label");
		edje_object_part_text_set(label, "default", str);
		edje_object_signal_callback_add(label, "in", MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _vertical_in, ui);
		edje_object_signal_callback_add(label, "out", MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _vertical_out, ui);
		edje_object_signal_callback_add(label, "changed", MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _vertical_changed, ui);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_show(label);
		evas_object_table_pack(ui->grid, label, 0x10, i, 1, 1);
		ui->tiles[0x10][i] = label;
	}

	Evas_Object *def = edje_object_add(ui->e);
	edje_object_file_set(def, buf, MIDI_MATRIX_CHANNEL_FILTER_UI_URI"/label");
	edje_object_part_text_set(def, "default", "D");
	edje_object_signal_callback_add(def, "in", MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _DAC_in, ui);
	edje_object_signal_callback_add(def, "out", MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _DAC_out, ui);
	edje_object_signal_callback_add(def, "changed", MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _def_changed, ui);
	evas_object_size_hint_weight_set(def, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(def, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(def);
	evas_object_table_pack(ui->grid, def, 0x11, 0x10, 1, 1);

	Evas_Object *all = edje_object_add(ui->e);
	edje_object_file_set(all, buf, MIDI_MATRIX_CHANNEL_FILTER_UI_URI"/label");
	edje_object_part_text_set(all, "default", "A");
	edje_object_signal_callback_add(all, "in", MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _DAC_in, ui);
	edje_object_signal_callback_add(all, "out", MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _DAC_out, ui);
	edje_object_signal_callback_add(all, "changed", MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _all_changed, ui);
	evas_object_size_hint_weight_set(all, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(all, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(all);
	evas_object_table_pack(ui->grid, all, 0x10, 0x11, 1, 1);

	Evas_Object *clear = edje_object_add(ui->e);
	edje_object_file_set(clear, buf, MIDI_MATRIX_CHANNEL_FILTER_UI_URI"/label");
	edje_object_part_text_set(clear, "default", "C");
	edje_object_signal_callback_add(clear, "in", MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _DAC_in, ui);
	edje_object_signal_callback_add(clear, "out", MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _DAC_out, ui);
	edje_object_signal_callback_add(clear, "changed", MIDI_MATRIX_CHANNEL_FILTER_UI_URI, _clear_changed, ui);
	evas_object_size_hint_weight_set(clear, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(clear, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(clear);
	evas_object_table_pack(ui->grid, clear, 0x11, 0x11, 1, 1);

	Evas_Object *input_label = edje_object_add(ui->e);
	edje_object_file_set(input_label, buf, MIDI_MATRIX_CHANNEL_FILTER_UI_URI"/label/inputs");
	edje_object_part_text_set(input_label, "default", "Input Channels");
	evas_object_size_hint_weight_set(input_label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(input_label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(input_label);
	evas_object_table_pack(ui->grid, input_label, 0, 0x11, 0x10, 1);

	Evas_Object *output_label = edje_object_add(ui->e);
	edje_object_file_set(output_label, buf, MIDI_MATRIX_CHANNEL_FILTER_UI_URI"/label/outputs");
	edje_object_part_text_set(output_label, "default", "Output Channels");
	evas_object_size_hint_weight_set(output_label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(output_label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(output_label);
	evas_object_table_pack(ui->grid, output_label, 0x11, 0, 1, 0x10);

	if(ui->ee) // X11 UI
		*(Evas_Object **)widget = NULL;
	else // Eo UI
		*(Evas_Object **)widget = ui->theme;
	
	return ui;
}

static void
cleanup(LV2UI_Handle handle)
{
	UI *ui = handle;
	
	if(ui)
	{
		if(ui->ee)
		{
			ecore_evas_hide(ui->ee);
			evas_object_del(ui->theme);
			ecore_evas_free(ui->ee);
		}
		
		free(ui);
	}

	edje_shutdown();
	ecore_evas_shutdown();
	evas_shutdown();
	ecore_shutdown();
	eina_shutdown();
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

const LV2UI_Descriptor lv2_midi_matrix_channel_filter_ui = {
	.URI						= MIDI_MATRIX_CHANNEL_FILTER_UI_URI,
	.instantiate		= instantiate,
	.cleanup				= cleanup,
	.port_event			= port_event,
	.extension_data	= extension_data
};

const LV2UI_Descriptor lv2_midi_matrix_channel_filter_eo = {
	.URI						= MIDI_MATRIX_CHANNEL_FILTER_EO_URI,
	.instantiate		= instantiate,
	.cleanup				= cleanup,
	.port_event			= port_event,
	.extension_data	= NULL
};
