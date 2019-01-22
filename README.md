# B.SEQuencer
Repository: BSEQuencer

Description: Multi channel 16x16 MIDI step sequencer LV2 plugin

Installation
------------
Build your own binaries in the following three steps.

Step 1: Clone or download this repository.

Step 2: Install the development packages for x11, cairo, and lv2 if not done yet.

Step 3: Building and installing into the default lv2 directory (/usr/lib/lv2/) is easy. Simply call:
```
sudo make install
```
from the directory where you downloaded the repository files.

For installation into an alternative directory (e.g., /usr/local/lib/lv2), modify line 2 in the makefile.

Running
-------
(After the installation Ardour, Carla, and any other LV2 host should automatically detect B.SEQuencer.)

If jalv is installed, you can also call it
```
jalv.gtk https://www.jahnichen.de/plugins/lv2/BSEQuencer
```
to run it stand-alone and connect it to the JACK system.

Key features
------------
* Step sequencer with a note-based 16x16 matrix
* Control panel (in line 1 of the matrix) with playing direction, jump, skip, and stop options
* Autoplay or host / MIDI controlled playing
* Handles multiple MIDI inputs signals (keys) in one sequencer instance
* Four configurable output channels
* Output channels connectable with individual MIDI ports / channels

Usage
-----
see https://github.com/sjaehn/BSEQuencer/wiki/B.SEQuencer

TODO
----
* Mouse over widgets (showing values of matrix fields, dials, or sliders; direct keybord keyboard entry of values)
* Stability (Note: **This plugin is experimental, yet. Not for production!**)
* Testing
* Bugfixes (a lot to do)
* Selectable number of steps (8, 16, 24, or 32 steps) - may cause incompatibility to previous versions
* Do not divide control fields from note fields - may cause incompatibility to previous versions

See also
--------
* https://www.youtube.com/watch?v=iERRKL7J-KU




