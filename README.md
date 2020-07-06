# B.SEQuencer
Repository: BSEQuencer

Description: Multi channel MIDI step sequencer LV2 plugin with a variable matrix

![screenshot](https://github.com/sjaehn/BSEQuencer/blob/master/docs/screenshot.png "Screenshot from B.SEQuencer")


Installation
------------
a) Install the bsequencer (bsequencer-lv2) package for your system
* [Arch linux](https://www.archlinux.org/packages/community/x86_64/bsequencer/) by David Runge
* [Free BSD](https://www.freshports.org/audio/bsequencer-lv2) by yurivict
* [NixOS](https://github.com/NixOS/nixpkgs/blob/master/pkgs/applications/audio/bsequencer/default.nix) by Bart Brouns
* [Debian](https://kx.studio/Repositories:Plugins) by falkTX
* Check https://repology.org/project/bsequencer/versions for other systems

b) Build your own binaries in the following three steps.

Step 1: Clone or download this repository.

Step 2: Install pkg-config and the development packages for x11, cairo, and lv2 if not done yet. On
Debian-based systems you may run:
```
sudo apt-get install pkg-config libx11-dev libcairo2-dev lv2-dev
```

Step 3: Building and installing into the default lv2 directory (/usr/lib/lv2/) is easy. Simply call:
```
make
sudo make install PREFIX=/usr
```
from the directory where you downloaded the repository files. For installation into an
alternative directory (e.g., /usr/local/lib/lv2/), change the variable `PREFIX` while installing:

```
sudo make install PREFIX=/usr/local
```


Running
-------
After the installation Carla, Ardour and any other LV2 host should automatically detect B.SEQuencer.

If jalv is installed, you can also call it
```
jalv.gtk https://www.jahnichen.de/plugins/lv2/BSEQuencer
```
to run it stand-alone and connect it to the JACK system.


Key features
------------
* Step sequencer with a selectable pattern matrix size (8x16, 16x16, 24x16, or 32x16)
* Autoplay or host / MIDI controlled playing
* Handles multiple MIDI inputs signals (keys) in one sequencer instance
* Use musical scales and / or drumkits
* Scale & drumkit editor
* Controls for playing direction, jump, skip, and stop options placeable to any position within the matrix
* Notes can be associated with four different, configurable output channels
* Output channels connectable with individual MIDI channels


What's new
----------
* Select between
  * Restart from step 0,
  * Restart from step 0 and sync with running steps,
  * Or continue from running step if key pressed
* Enable MIDI Thru (important for downstream generator plugins!)
* Factory presets added (Techno, Standard Rock, Jazz, and Disco drumkits)
* Compiler error with gcc10 fixed


Usage
-----
see https://github.com/sjaehn/BSEQuencer/wiki/B.SEQuencer


See also
--------
* Feature tour: https://www.youtube.com/watch?v=J6bU4GdUVYc
* Preview: https://www.youtube.com/watch?v=iERRKL7J-KU
