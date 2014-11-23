# Midi Matrix

## A LV2 Plugin Bundle

### Build status

[![Build Status](https://travis-ci.org/OpenMusicKontrollers/midi_matrix.lv2.svg)](https://travis-ci.org/OpenMusicKontrollers/midi_matrix.lv2)

### Plugins

#### Channel Filter

The Midi Matrix - Channel Filter is a 3-in-1 filter plugin with a simple UI enabling you to easily accomplish:
- MIDI channel filtering (e.g. blocking of specific channels)
- MIDI channel multiplication (e.g. send events from channel X to channels X, Y and Z)
- MIDI channel rerouting (e.g send events from channel X to channel Y)

![Preview](midi_matrix_channel_filter_preview.png)

### Dependencies

* [LV2](http://lv2plug.in) (LV2 Plugin Standard)
* [EFL](http://enlightenment.org) (Enlightenment Foundation Libraries)

### Build / install

	git clone https://github.com/OpenMusicKontrollers/midi_matrix.lv2.git
	cd midi_matrix.lv2
	mkdir build
	cd build
	cmake -DCMAKE_C_FLAGS="-std=gnu99" ..
	make
	sudo make install
