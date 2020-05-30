---
title: Export
---

# {{page.title}}

After a simulation has been completed, the data can be exported to file by right clicking on the image and selecting `Export` and then one of `RGB` or `Data`. Selecting `RGB` will let you save a .bmp image that is easily opened in any image viewer/editor, but the data will be scaled 8-bit (i.e. it will be integer values in the range 0-255). Selecting `Data` will let you save a full 32-bit floating point .tif file. This file will not open in many image viewers/editors but contains the full accuracy of the simulation. These images can however be opened in programs such as ImageJ and the Gatan Microscopy Suite.

Whenever you save an image, a partnet .json file will be created containing all of the relevant parameters used in the simulation.