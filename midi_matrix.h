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
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"
	
#define MIDI_MATRIX_URI											"http://open-music-kontrollers.ch/lv2/midi_matrix"

#define MIDI_MATRIX_CHANNEL_FILTER_URI			MIDI_MATRIX_URI"#channel_filter"
#define MIDI_MATRIX_CHANNEL_FILTER_UI_URI		MIDI_MATRIX_CHANNEL_FILTER_URI"/ui"
#define MIDI_MATRIX_CHANNEL_FILTER_EO_URI		MIDI_MATRIX_CHANNEL_FILTER_URI"/eo"

const LV2_Descriptor lv2_midi_matrix_channel_filter;

const LV2UI_Descriptor lv2_midi_matrix_channel_filter_ui;
const LV2UI_Descriptor lv2_midi_matrix_channel_filter_eo;

#endif // _MIDI_MATRIX_LV2_H
