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

	uint16_t mask [0x10];
};

static UI *ui_ptr;

static int
idle_cb(LV2UI_Handle handle)
{
	UI *ui = handle;

	if(ecore_main_loop_animator_ticked_get() == EINA_TRUE)
		ecore_animator_custom_tick();
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
_begin_tick(void *data)
{
	printf("begin_tick\n");
	ecore_animator_custom_tick();
}

static void
_end_tick(void *data)
{
	// do nothing
}

static LV2UI_Handle
instantiate(const LV2UI_Descriptor *descriptor, const char *plugin_uri, const char *bundle_path, LV2UI_Write_Function write_function, LV2UI_Controller controller, LV2UI_Widget *widget, const LV2_Feature *const *features)
{
	eina_init();
	ecore_init();
	evas_init();
	ecore_evas_init();
	edje_init();
	
	if(strcmp(plugin_uri, "http://open-music-kontrollers.ch/midi_matrixplex#channel"))
		return NULL;

	UI *ui = calloc(1, sizeof(UI));
	if(!ui)
		return NULL;

	ui->w = 400;
	ui->h = 225;
	ui->write_function = write_function;
	ui->controller = controller;

	void *parent = NULL;
	LV2UI_Resize *resize = NULL;
	
	int i;
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
	evas_object_resize(ui->bg, ui->w, ui->h);
	evas_object_show(ui->bg);

	ui->grid = evas_object_table_add(ui->e);
	evas_object_resize(ui->grid, ui->w, ui->h);
	evas_object_show(ui->grid);

	// FIXME add tiles

	ecore_main_loop_iterate();
	ecore_animator_custom_source_tick_begin_callback_set(_begin_tick, ui);
	ecore_animator_custom_source_tick_end_callback_set(_end_tick, ui);
	ecore_animator_source_set(ECORE_ANIMATOR_SOURCE_CUSTOM);
	
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
port_event(LV2UI_Handle handle, uint32_t port_index, uint32_t buffer_size, uint32_t format, const void *buffer)
{
	UI *ui = handle;

	switch(port_index)
	{
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
			ui->mask[port_index] = (uint16_t)*(const float *)buffer;
			break;
		default:
			break;
	}
	//TODO
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
	.URI						= "http://open-music-kontrollers.ch/midi_matrixplex#channel/ui",
	.instantiate		= instantiate,
	.cleanup				= cleanup,
	.port_event			= port_event,
	.extension_data	= extension_data
};
