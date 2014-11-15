#ifndef _MIDI_MATRIXPLEX_LV2_H
#define _MIDI_MATRIXPLEX_LV2_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <endian.h>

#include "lv2/lv2plug.in/ns/ext/atom/atom.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/atom/forge.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"
	
#define MIDI_MATRIXPLEX_URI "http://open-music-kontrollers.ch/midi_matrixplex"

#define MIDI_MATRIXPLEX_CHANNEL_URI MIDI_MATRIXPLEX_URI"#channel"
#define MIDI_MATRIXPLEX_CHANNEL_UI_URI MIDI_MATRIXPLEX_CHANNEL_URI"/ui"

#endif // _MIDI_MATRIXPLEX_LV2_H
