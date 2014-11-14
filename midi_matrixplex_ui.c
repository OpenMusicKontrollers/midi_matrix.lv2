#include <midi_matrixplex.h>

extern const LV2UI_Descriptor lv2_midi_matrixplex_channel_ui;

LV2_SYMBOL_EXPORT const LV2UI_Descriptor*
lv2ui_descriptor(uint32_t index)
{
	switch(index)
	{
		case 0:
			return &lv2_midi_matrixplex_channel_ui;
		default:
			return NULL;
	}
}
