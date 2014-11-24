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
- And any possible combination thereof

##### Example Configurations

![Preview 1](2014-11-24_104234_374410229.png "Through: Connection 03-03 highlighted")

_Through: Connection 03-03 highlighted_

![Preview 2](2014-11-24_104256_209772357.png "Block: All channels blocked but channel 01")

_Block: All channels blocked but channel 01_

![Preview 3](2014-11-24_104333_431936314.png "Multiplication: Send channel 01 events to all channels")

_Multiplication: Send channel 01 events to all channels_

![Preview 4](2014-11-24_104415_970283927.png "Multiplex: Aggregate events from all channels to channel 02")

_Multiplex: Aggregate events from all channels to channel 02_

![Preview 5](2014-11-24_104434_366927757.png "Reroute: Switch events of port 01/02 and 03/04, respectively")

_Reroute: Switch events of port 01/02 and 03/04, respectively_

![Preview 6](2014-11-24_104556_312064039.png "Reroute/Multiplex: Aggregate events from odd ports to channel 01 and events from even ports to channel 02")

_Reroute/Multiplex: Aggregate events from odd ports to channel 01 and events from even ports to channel 02_

![Preview 7](2014-11-24_104618_837590650.png "Default: The default configuration is a Midi-Through")

_Default: The default configuration is a Midi-Through_

![Preview 8](2014-11-24_104625_534545236.png "Clear: All channels are blocked")

_Clear: All channels are blocked_

![Preview 9](2014-11-24_104632_038450650.png "All: Route everything to everyghing")

_All: Route everything to everyghing_


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
