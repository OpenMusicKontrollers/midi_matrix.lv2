## Midi Matrix

### An LV2 MIDI channel matrix patcher

Things that can be accomplished with this plugin:

* MIDI channel filtering (e.g. blocking of specific channels)
* MIDI channel multiplication (e.g. send events from channel X to channels X, Y and Z)
* MIDI channel rerouting (e.g send events from channel X to channel Y)
* And any possible combination thereof

#### Build status

[![build status](https://gitlab.com/OpenMusicKontrollers/midi_matrix.lv2/badges/master/build.svg)](https://gitlab.com/OpenMusicKontrollers/midi_matrix.lv2/commits/master)

### Binaries

For GNU/Linux (64-bit, 32-bit, armv7), Windows (64-bit, 32-bit) and MacOS
(64/32-bit univeral).

To install the plugin bundle on your system, simply copy the __midi_matrix.lv2__
folder out of the platform folder of the downloaded package into your
[LV2 path](http://lv2plug.in/pages/filesystem-hierarchy-standard.html).

#### Stable release

* [midi_matrix.lv2-0.20.0.zip](https://dl.open-music-kontrollers.ch/midi_matrix.lv2/stable/midi_matrix.lv2-0.20.0.zip) ([sig](https://dl.open-music-kontrollers.ch/midi_matrix.lv2/stable/midi_matrix.lv2-0.20.0.zip.sig))

#### Unstable (nightly) release

* [midi_matrix.lv2-latest-unstable.zip](https://dl.open-music-kontrollers.ch/midi_matrix.lv2/unstable/midi_matrix.lv2-latest-unstable.zip) ([sig](https://dl.open-music-kontrollers.ch/midi_matrix.lv2/unstable/midi_matrix.lv2-latest-unstable.zip.sig))

### Sources

#### Stable release

* [midi_matrix.lv2-0.20.0.tar.xz](https://git.open-music-kontrollers.ch/lv2/midi_matrix.lv2/snapshot/midi_matrix.lv2-0.20.0.tar.xz)

#### Git repository

* <https://git.open-music-kontrollers.ch/lv2/midi_matrix.lv2>

### Packages

* [ArchLinux](https://www.archlinux.org/packages/community/x86_64/midi_matrix.lv2/)

### Bugs and feature requests

* [Gitlab](https://gitlab.com/OpenMusicKontrollers/midi_matrix.lv2)
* [Github](https://github.com/OpenMusicKontrollers/midi_matrix.lv2)

### Plugins

### Channel Filter

![Through](https://git.open-music-kontrollers.ch/lv2/midi_matrix.lv2/plain/screenshots/screenshot_1.png)

The _Midi Matrix - Channel Filter_ is a 3-in-1 filter plugin with a simple UI enabling you to easily accomplish:

* MIDI channel filtering (e.g. blocking of specific channels)
* MIDI channel multiplication (e.g. send events from channel X to channels X, Y and Z)
* MIDI channel rerouting (e.g send events from channel X to channel Y)
* And any possible combination thereof

#### Dependencies

* [LV2](http://lv2plug.in) (LV2 Plugin Specification)

#### Build / install

	git clone https://git.open-music-kontrollers.ch/lv2/midi_matrix.lv2.git
	cd midi_matrix.lv2
	meson build
	cd build
	ninja -j4
	sudo ninja install

#### License

Copyright (c) 2015-2016 Hanspeter Portner (dev@open-music-kontrollers.ch)

This is free software: you can redistribute it and/or modify
it under the terms of the Artistic License 2.0 as published by
The Perl Foundation.

This source is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
Artistic License 2.0 for more details.

You should have received a copy of the Artistic License 2.0
along the source as a COPYING file. If not, obtain it from
<http://www.perlfoundation.org/artistic_license_2_0>.
