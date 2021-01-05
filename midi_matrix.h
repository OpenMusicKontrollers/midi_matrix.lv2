/*
 * Copyright (c) 2015-2020 Hanspeter Portner (dev@open-music-kontrollers.ch)
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

#ifndef _MIDI_MATRIX_LV2_H
#define _MIDI_MATRIX_LV2_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "lv2/lv2plug.in/ns/ext/atom/atom.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/atom/forge.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/ext/options/options.h"
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"
	
#define MIDI_MATRIX_URI											"http://open-music-kontrollers.ch/lv2/midi_matrix"

#define MIDI_MATRIX_CHANNEL_FILTER_URI			MIDI_MATRIX_URI"#channel_filter"

#define MIDI_MATRIX_CHANNEL_FILTER_NK_URI		MIDI_MATRIX_URI"#channel_filter_4_nk"

extern const LV2_Descriptor channel_filter;

extern const LV2UI_Descriptor channel_filter_nk;

#endif // _MIDI_MATRIX_LV2_H
