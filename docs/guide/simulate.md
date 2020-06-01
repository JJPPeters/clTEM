---
title: Simulating
---

# {{page.title}}

Simulating is as easy as setting up the configuration as you need (or load a configuration file) and pressing the `Sim` button on the bottom-right, or by clicking the <code>Simulation &rarr; Simulate</code> menu entry. A warning dialog will appear if some critical parameter has not been set (i.e. if a structure hasn't been loaded).

During the simulation, the progress will be shown via the progress bars in the bottom-right. The botom bar shows the total simulation progress and the top bar shows the progress per simulation part (i.e. if doing STEM, it will show the progress for the current pixel). If multiple devices are being used, this top bar will behave erratically as all devices will try to update this with their own progress.

When a simulation is complete, the results will be shown in the appropriate tab of the Output panel. 