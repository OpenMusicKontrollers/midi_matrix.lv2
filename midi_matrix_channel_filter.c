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

typedef struct _Handle Handle;

struct _Handle {
	LV2_URID_Map *map;
	struct {
		LV2_URID midi_MidiEvent;
	} uris;

	const LV2_Atom_Sequence *midi_in;
	LV2_Atom_Sequence *midi_out;
	float *control [0x10];
	uint16_t mask [0x10];
	
	LV2_Atom_Forge forge;
	LV2_Atom_Forge_Frame frame;
};

static LV2_Handle
instantiate(const LV2_Descriptor* descriptor, double rate, const char *bundle_path, const LV2_Feature *const *features)
{
	int i;
	Handle *handle = (Handle *)calloc(1, sizeof(Handle));
	if(!handle)
		return NULL;

	for(i=0; features[i]; i++)
		if(!strcmp(features[i]->URI, LV2_URID__map))
			handle->map = (LV2_URID_Map *)features[i]->data;

	if(!handle->map)
	{
		fprintf(stderr, "%s: Host does not support urid:map\n", descriptor->URI);
		free(handle);
		return NULL;
	}

	handle->uris.midi_MidiEvent = handle->map->map(handle->map->handle, LV2_MIDI__MidiEvent);
	lv2_atom_forge_init(&handle->forge, handle->map);

	return handle;
}

static void
connect_port(LV2_Handle instance, uint32_t port, void *data)
{
	Handle *handle = (Handle *)instance;
	switch(port)
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
			handle->control[port] = (float *)data;
			break;
		case 0x10:
			handle->midi_in = (const LV2_Atom_Sequence *)data;
			break;
		case 0x11:
			handle->midi_out = (LV2_Atom_Sequence *)data;
			break;
		default:
			break;
	}
}

static void
activate(LV2_Handle instance)
{
	Handle *handle = (Handle *)instance;
	//nothing
}

static void
run(LV2_Handle instance, uint32_t sample_count)
{
	Handle *handle = (Handle *)instance;

	if(!handle->midi_in || !handle->midi_out)
		return;

	// fill channel mask array
	int i;
	for(i=0x0; i<0x10; i++)
		handle->mask[i] = (uint16_t)*handle->control[i];

	// prepare midi atom forge
	const uint32_t capacity = handle->midi_out->atom.size;
	LV2_Atom_Forge *forge = &handle->forge;
	lv2_atom_forge_set_buffer(forge, (uint8_t *)handle->midi_out, capacity);
	lv2_atom_forge_sequence_head(forge, &handle->frame, 0);

	// process incoming events
	LV2_ATOM_SEQUENCE_FOREACH(handle->midi_in, ev)
	{
		if(ev->body.type == handle->uris.midi_MidiEvent)
		{
			int64_t frames = ev->time.frames;
			const uint32_t len = ev->body.size;
			const uint8_t *buf = LV2_ATOM_BODY_CONST(&ev->body);

			const uint8_t cmd = buf[0] & 0xf0;
			if(cmd == 0xf0)
				continue; // ignore system messages

			const uint8_t src = buf[0] & 0x0f; // source channel
			if(handle->mask[src]) // are there any active output channels at all for this input channel?
			{
				uint8_t dst;
				uint16_t mask;
				for(dst=0x0, mask=0x1; dst<0x10; dst++, mask=mask<<1)
				{
					if(handle->mask[src] & mask) // is this output channel active?
					{
						LV2_Atom midiatom;
						midiatom.type = handle->uris.midi_MidiEvent;
						midiatom.size = len;
						const uint8_t m = (buf[0] & 0xf0) | dst; // rewrite channel number
						
						lv2_atom_forge_frame_time(forge, frames);
						lv2_atom_forge_raw(forge, &midiatom, sizeof(LV2_Atom));
						lv2_atom_forge_raw(forge, &m, 1);
						lv2_atom_forge_raw(forge, &buf[1], len-1);
						lv2_atom_forge_pad(forge, sizeof(LV2_Atom) + len);
					}
				}
			}
		}
	}

	lv2_atom_forge_pop(forge, &handle->frame);
}

static void
deactivate(LV2_Handle instance)
{
	Handle *handle = (Handle *)instance;
	//nothing
}

static void
cleanup(LV2_Handle instance)
{
	Handle *handle = (Handle *)instance;

	free(handle);
}

static const void*
extension_data(const char* uri)
{
	//nothing
	return NULL;
}

const LV2_Descriptor channel_filter = {
	.URI						= MIDI_MATRIX_CHANNEL_FILTER_URI,
	.instantiate		= instantiate,
	.connect_port		= connect_port,
	.activate				= activate,
	.run						= run,
	.deactivate			= deactivate,
	.cleanup				= cleanup,
	.extension_data	= extension_data
};
