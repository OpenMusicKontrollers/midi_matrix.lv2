#include <midi_matrixplex.h>

#include <Eina.h>
#include <Ecore.h>
#include <Evas.h>
#include <Ecore_Evas.h>
#include <Edje.h>

typedef struct _UI UI;

struct _UI {
	LV2UI_Write_Function write_function;
	LV2UI_Controller controller;
	LV2UI_Idle_Interface idle_ext;
	LV2UI_Resize resize_ext;

	int w, h;
	Ecore_Evas *ee;
	Evas *e;
	Evas_Object *bg;
	Evas_Object *grid;
	Evas_Object *tiles [0x11][0x11];
	
	uint16_t mask [0x10];
};

static UI *ui_ptr;

static int
idle_cb(LV2UI_Handle handle)
{
	UI *ui = handle;

	ecore_main_loop_iterate();
	
	return 0;
}

static int
resize_cb(LV2UI_Feature_Handle handle, int w, int h)
{
	UI *ui = handle;

	if(!ui)
		return -1;

	ui->w = w;
	ui->h = h;

	ecore_evas_resize(ui->ee, ui->w, ui->h);
	evas_object_resize(ui->bg, ui->w, ui->h);
	evas_object_resize(ui->grid, ui->w, ui->h);
  
  return 0;
}

static void
_in(void *data, Evas_Object *edj, const char *emission, const char *source)
{
	UI *ui = data;
	int i, j;
	
	unsigned short x, y, w, h;
	evas_object_table_pack_get(ui->grid, edj, &x, &y, &w, &h);

	edje_object_signal_emit(ui->tiles[x][y], "edge", MIDI_MATRIXPLEX_CHANNEL_UI_URI);
	if( (x==0x10) || (y==0x10) ) // is label
	{
		if(y==0x10)
		{
			int vert = 0;
			uint16_t mask = ui->mask[x];
			uint16_t dst;
			for(i=0, dst=1; i<0x10; i++, dst=dst<<1)
				if(mask & dst)
				{
					edje_object_signal_emit(ui->tiles[x][i], "highlight,on", MIDI_MATRIXPLEX_CHANNEL_UI_URI);
					if(!vert)
					{
						edje_object_signal_emit(ui->tiles[x][i], "edge", MIDI_MATRIXPLEX_CHANNEL_UI_URI);
						for(j=0x10; j>i; j--)
							edje_object_signal_emit(ui->tiles[x][j], "vertical", MIDI_MATRIXPLEX_CHANNEL_UI_URI);
						vert = 1;
					}
					else // vert
						edje_object_signal_emit(ui->tiles[x][i], "edge,vertical", MIDI_MATRIXPLEX_CHANNEL_UI_URI);
					for(j=0x10; j>x; j--)
						edje_object_signal_emit(ui->tiles[j][i], "horizontal", MIDI_MATRIXPLEX_CHANNEL_UI_URI);
				}
		}
		else // (x==0x10)
		{
			int horz = 0;
			uint16_t mask = 1<<y;
			for(i=0; i<0x10; i++)
				if(mask & ui->mask[i])
				{
					edje_object_signal_emit(ui->tiles[i][y], "highlight,on", MIDI_MATRIXPLEX_CHANNEL_UI_URI);
					if(!horz)
					{
						edje_object_signal_emit(ui->tiles[i][y], "edge", MIDI_MATRIXPLEX_CHANNEL_UI_URI);
						for(j=0x10; j>i; j--)
							edje_object_signal_emit(ui->tiles[j][y], "horizontal", MIDI_MATRIXPLEX_CHANNEL_UI_URI);

						horz = 1;
					}
					else // horz
						edje_object_signal_emit(ui->tiles[i][y], "edge,horizontal", MIDI_MATRIXPLEX_CHANNEL_UI_URI);
					for(j=0x10; j>y; j--)
						edje_object_signal_emit(ui->tiles[i][j], "vertical", MIDI_MATRIXPLEX_CHANNEL_UI_URI);
				}
		}
	}
	else
	{
		for(i=0x10; i>x; i--)
			edje_object_signal_emit(ui->tiles[i][y], "horizontal", MIDI_MATRIXPLEX_CHANNEL_UI_URI);
		for(i=0x10; i>y; i--)
			edje_object_signal_emit(ui->tiles[x][i], "vertical", MIDI_MATRIXPLEX_CHANNEL_UI_URI);
	}
}

static void
_out(void *data, Evas_Object *edj, const char *emission, const char *source)
{
	UI *ui = data;
	int i, j;
	
	unsigned short x, y, w, h;
	evas_object_table_pack_get(ui->grid, edj, &x, &y, &w, &h);

	edje_object_signal_emit(ui->tiles[x][y], "clear", MIDI_MATRIXPLEX_CHANNEL_UI_URI);
	if( (x==0x10) || (y==0x10) ) // is label
	{
		if(y==0x10)
		{
			int vert = 0;
			uint16_t mask = ui->mask[x];
			uint16_t dst;
			for(i=0, dst=1; i<0x10; i++, dst=dst<<1)
				if(mask & dst)
				{
					edje_object_signal_emit(ui->tiles[x][i], "highlight,off", MIDI_MATRIXPLEX_CHANNEL_UI_URI);
					if(!vert)
					{
						edje_object_signal_emit(ui->tiles[x][i], "clear", MIDI_MATRIXPLEX_CHANNEL_UI_URI);
						for(j=0x10; j>i; j--)
							edje_object_signal_emit(ui->tiles[x][j], "clear", MIDI_MATRIXPLEX_CHANNEL_UI_URI);
						vert = 1;
					}
					else // vert
						edje_object_signal_emit(ui->tiles[x][i], "clear", MIDI_MATRIXPLEX_CHANNEL_UI_URI);
					for(j=0x10; j>x; j--)
						edje_object_signal_emit(ui->tiles[j][i], "clear", MIDI_MATRIXPLEX_CHANNEL_UI_URI);
				}
		}
		else
		{
			int horz = 0;
			uint16_t mask = 1<<y;
			for(i=0; i<0x10; i++)
				if(mask & ui->mask[i])
				{
					edje_object_signal_emit(ui->tiles[i][y], "highlight,off", MIDI_MATRIXPLEX_CHANNEL_UI_URI);
					if(!horz)
					{
						edje_object_signal_emit(ui->tiles[i][y], "clear", MIDI_MATRIXPLEX_CHANNEL_UI_URI);
						for(j=0x10; j>i; j--)
							edje_object_signal_emit(ui->tiles[j][y], "clear", MIDI_MATRIXPLEX_CHANNEL_UI_URI);

						horz = 1;
					}
					else // horz
						edje_object_signal_emit(ui->tiles[i][y], "clear", MIDI_MATRIXPLEX_CHANNEL_UI_URI);
					for(j=0x10; j>y; j--)
						edje_object_signal_emit(ui->tiles[i][j], "clear", MIDI_MATRIXPLEX_CHANNEL_UI_URI);
				}
		}
	}
	else
	{
		for(i=0x10; i>x; i--)
			edje_object_signal_emit(ui->tiles[i][y], "clear", MIDI_MATRIXPLEX_CHANNEL_UI_URI);
		for(i=0x10; i>y; i--)
			edje_object_signal_emit(ui->tiles[x][i], "clear", MIDI_MATRIXPLEX_CHANNEL_UI_URI);
	}
}

static uint16_t
_refresh(UI *ui, int i, uint16_t src, uint16_t dst)
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
					edje_object_signal_emit(ui->tiles[i][j], "on", MIDI_MATRIXPLEX_CHANNEL_UI_URI);
				else
					edje_object_signal_emit(ui->tiles[i][j], "off", MIDI_MATRIXPLEX_CHANNEL_UI_URI);
			}
	}

	return dst;
}

static void
_changed(void *data, Evas_Object *edj, const char *emission, const char *source)
{
	UI *ui = data;

	unsigned short i, j, w, h;
	evas_object_table_pack_get(ui->grid, edj, &i, &j, &w, &h);

	uint16_t *src = &ui->mask[i];
	uint16_t mask = 1 << j;
	if(*src & mask) // on -> off
		*src = _refresh(ui, i, *src, *src & ~mask);
	else // off -> on
		*src = _refresh(ui, i, *src, *src | mask);

	float control = *src;
	ui->write_function(ui->controller, i, sizeof(float), 0, &control);
}

static LV2UI_Handle
instantiate(const LV2UI_Descriptor *descriptor, const char *plugin_uri, const char *bundle_path, LV2UI_Write_Function write_function, LV2UI_Controller controller, LV2UI_Widget *widget, const LV2_Feature *const *features)
{
	eina_init();
	ecore_init();
	evas_init();
	ecore_evas_init();
	edje_init();

	edje_frametime_set(0.04);

	if(strcmp(plugin_uri, MIDI_MATRIXPLEX_CHANNEL_URI))
		return NULL;

	UI *ui = calloc(1, sizeof(UI));
	if(!ui)
		return NULL;

	ui->w = 30 * 0x11;
	ui->h = 30 * 0x11;
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

	ui->ee = ecore_evas_gl_x11_new(NULL, (Ecore_X_Window)parent, 0, 0, ui->w, ui->h);
	if(!ui->ee)
		ui->ee = ecore_evas_software_x11_new(NULL, (Ecore_X_Window)parent, 0, 0, ui->w, ui->h);
	if(!ui->ee)
		printf("could not start evas\n");
	ui->e = ecore_evas_get(ui->ee);
	ecore_evas_show(ui->ee);

	if(resize)
    resize->ui_resize(resize->handle, ui->w, ui->h);

	ui->bg = evas_object_rectangle_add(ui->e);
	evas_object_color_set(ui->bg, 48, 48, 48, 255);
	evas_object_resize(ui->bg, ui->w, ui->h);
	evas_object_show(ui->bg);

	ui->grid = evas_object_table_add(ui->e);
	evas_object_resize(ui->grid, ui->w, ui->h);
	evas_object_show(ui->grid);

	char buf [256];
	sprintf(buf, "%s/midi_matrixplex.edj", bundle_path);

	for(i=0; i<0x10; i++)
	{
		for(j=0; j<0x10; j++)
		{
			Evas_Object *tile = edje_object_add(ui->e);
			edje_object_file_set(tile, buf, MIDI_MATRIXPLEX_CHANNEL_UI_URI"/tile");
			edje_object_signal_callback_add(tile, "in", MIDI_MATRIXPLEX_CHANNEL_UI_URI, _in, ui);
			edje_object_signal_callback_add(tile, "out", MIDI_MATRIXPLEX_CHANNEL_UI_URI, _out, ui);
			edje_object_signal_callback_add(tile, "changed", MIDI_MATRIXPLEX_CHANNEL_UI_URI, _changed, ui);
			evas_object_size_hint_weight_set(tile, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_size_hint_align_set(tile, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_show(tile);
			evas_object_table_pack(ui->grid, tile, i, j, 1, 1);
			ui->tiles[i][j] = tile;
		}

		char str [3];
		Evas_Object *label;

		sprintf(str, "i%01X", i);
		label = edje_object_add(ui->e);
		edje_object_file_set(label, buf, MIDI_MATRIXPLEX_CHANNEL_UI_URI"/label");
		edje_object_part_text_set(label, "default", str);
		edje_object_signal_callback_add(label, "in", MIDI_MATRIXPLEX_CHANNEL_UI_URI, _in, ui);
		edje_object_signal_callback_add(label, "out", MIDI_MATRIXPLEX_CHANNEL_UI_URI, _out, ui);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_show(label);
		evas_object_table_pack(ui->grid, label, i, 0x10, 1, 1);
		ui->tiles[i][0x10] = label;

		sprintf(str, "o%01X", i);
		label = edje_object_add(ui->e);
		edje_object_file_set(label, buf, MIDI_MATRIXPLEX_CHANNEL_UI_URI"/label");
		edje_object_part_text_set(label, "default", str);
		edje_object_signal_callback_add(label, "in", MIDI_MATRIXPLEX_CHANNEL_UI_URI, _in, ui);
		edje_object_signal_callback_add(label, "out", MIDI_MATRIXPLEX_CHANNEL_UI_URI, _out, ui);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_show(label);
		evas_object_table_pack(ui->grid, label, 0x10, i, 1, 1);
		ui->tiles[0x10][i] = label;
	}
	
	ui->idle_ext.idle = idle_cb;
	ui->resize_ext.handle = ui;
	ui->resize_ext.ui_resize = resize_cb;

	ui_ptr = ui; // FIXME bloddy hack

	return ui;
}

static void
cleanup(LV2UI_Handle handle)
{
	UI *ui = handle;
	
	if(ui)
	{
		ecore_evas_hide(ui->ee);

		int i, j;
		for(i=0; i<0x10; i++)
		{
			for(j=0; j<0x10; j++)
				evas_object_del(ui->tiles[i][j]);

			evas_object_del(ui->tiles[i][0x10]);
			evas_object_del(ui->tiles[0x10][i]);
		}
		evas_object_del(ui->grid);
		evas_object_del(ui->bg);
		ecore_evas_free(ui->ee);
		
		free(ui);
	}

	edje_init();
	ecore_evas_init();
	evas_init();
	ecore_init();
	eina_init();
}

static void
port_event(LV2UI_Handle handle, uint32_t i, uint32_t buffer_size, uint32_t format, const void *buffer)
{
	UI *ui = handle;

	if(i < 0x10)
	{
		uint16_t *src = &ui->mask[i];
		uint16_t dst = (uint16_t)*(const float *)buffer;

		*src = _refresh(ui, i, *src, dst);
	}
}

static const void *
extension_data(const char *uri)
{
	if (!strcmp(uri, LV2_UI__idleInterface))
		return &ui_ptr->idle_ext;
	if (!strcmp(uri, LV2_UI__resize))
		return &ui_ptr->resize_ext;
		
	return NULL;
}

const LV2UI_Descriptor lv2_midi_matrixplex_channel_ui = {
	.URI						= MIDI_MATRIXPLEX_CHANNEL_UI_URI,
	.instantiate		= instantiate,
	.cleanup				= cleanup,
	.port_event			= port_event,
	.extension_data	= extension_data
};
