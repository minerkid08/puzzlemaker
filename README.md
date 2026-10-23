# Overview
A portal 2 puzzlemaker similer to the ingame one
This project is a work in progress

This puzzlemaker has a 64 hammer unit grid which is the size of one medium tile.
Items can be placed anywhere and dont have to be snapped to the grid and can be rotated to any angle.
Antlines are manually routed right now and automatically generate a corner at the start of each segment.
Everything so far is highly configureable through packages like items and themes. Themes can change every texture in the editor and resize antlines to fit any style. Each theme has a set of post processing presets which can improve the look of your chambers.

Currently there is a limited selection of items compared to the portal 2 puzzlemaker, however more items and themes can be added through custom packages similar to how beemod works.
Docs for the item, package and themes and export script systems are in the docs directory.

# Installation
1. Download the latest release and unzip it.
2. If your p2ce installation is not in the default location or you are on linux you will need to modify the p2ce field inside compileSteps.json to point to your game installation.

# Key binds
Move with wasd and the arrow keys like hammer or the mouse like the portal 2 puzzlemaker.  
Press 2 to push voxels  
Press 3 to pull voxels  
Press R to toggle portalibility  
Press Z to toggle mini tiles  
  
Press G to grab item to be moved with mouse  
Press F to rotate item with mouse  
Press Q to rotate item 90 deg left  
Press E to rotate item 90 deg right  
