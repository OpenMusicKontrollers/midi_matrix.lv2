#include <midi_matrixplex.h>

extern const LV2_Descriptor lv2_midi_matrixplex_channel;

LV2_SYMBOL_EXPORT const LV2_Descriptor*
lv2_descriptor(uint32_t index)
{
	switch(index)
	{
		case 0:
			return &lv2_midi_matrixplex_channel;
		default:
			return NULL;
	}
}
