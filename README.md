# Midi Matrix

## A LV2 Plugin Bundle

### Webpage 

Get more information at: [http://open-music-kontrollers.ch/lv2/midi_matrix](http://open-music-kontrollers.ch/lv2/midi_matrix)

### Build status

[![Build Status](https://travis-ci.org/OpenMusicKontrollers/midi_matrix.lv2.svg)](https://travis-ci.org/OpenMusicKontrollers/midi_matrix.lv2)

### Plugins

#### Channel Filter

The Midi Matrix - Channel Filter is a 3-in-1 filter plugin with a simple UI enabling you to easily accomplish:
- MIDI channel filtering (e.g. blocking of specific channels)
- MIDI channel multiplication (e.g. send events from channel X to channels X, Y and Z)
- MIDI channel rerouting (e.g send events from channel X to channel Y)
- And any possible combination thereof

##### Example Configuration

![Preview 1](http://open-music-kontrollers.ch/lv2/midi_matrix/2014-11-24_104234_374410229.png "Through: Connection 03-03 highlighted")

_Through: Connection 03-03 highlighted_

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
