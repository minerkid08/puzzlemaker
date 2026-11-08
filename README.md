# Overview
A puzzlemaker for Portal 2: Community Edition

This puzzlemaker has a 64 hammer unit grid which is the size of one medium tile.
Items can be placed anywhere and dont have to be snapped to the grid and can be rotated to any angle.
Antlines are manually routed and can optionally generate a corner at the start of each segment.

Everything is highly configureable through packages.
Items are seperated by package then they are grouped to make it easy to find what you are looking for even with a large amount of items.  
There are several types of items: Entity, Panel, Volume and Overlay.
 - Entity items are point items and can compile to entities or instances.
 - Panel items are scalable, and can have up to 8 border components and props on the edge of each border, good for fizzlers and glass or grating.
 - Volume items are also scalable and can have different textures on each face, good for triggers and toxic goo.
 - Overlay items are overlays that can be placed on voxels and the faces of some volume items, good for test chamber signage.
All items can have export scripts attached for more complex items.

Themes can change every texture in the editor and resize antlines to fit any style.
Each theme has a set of post processing presets which can improve the look of your chambers.

Currently there is a limited selection of items compared to the portal 2 puzzlemaker, however more items and themes can be added through custom packages similar to how beemod works.
Docs for the item, package and themes and export script systems are in the docs directory.

# Installation
1. Download the latest release and unzip it.
2. If your P2CE installation is not in the default location `C:\Program Files (x86)\Steam\steamapps\common\Portal 2 Community Edition` or you are on linux you will need to modify the p2ce field inside compileSteps.json to point to your game installation.

# Key binds
Some of the basic key binds are:
 - 2 to push voxels
 - 3 to pull voxels
 - R to toggle portalibility
 - G to grab item to be moved with mouse
 - Q to rotate item 90 deg left
 - E to rotate item 90 deg right
 - C to open item placement menu
For a full list of key binds open the keybinds window at window -> keybinds

# Community Packages
### Portal 1 Theme and Helper Items by Crazy.Hair
[Github](https://github.com/crazy-hair/p2ce-puzzlemaker-assets)
![alt](images/p1pbr.png)
