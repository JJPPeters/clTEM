---
title: Configuration
---

# {{page.title}}

The simulation configuration can be set using the GUI or via a .json file. The .json can set every parameter, so it is daunting to go through every option available. The easiest way to create a .json configuration file is to export one from the GUI. Using the <code>File &rarr; Config... &rarr; Export</code> menu, the current settings in the GUI will be saved to the defined .json file. This will contain every option available if you wish to go through and edit it yourself.

**Note** the configuration file has entries for units. These are purely descriptive, and do not define how clTEM will read in the values (i.e. if you change Å to nm, the value will still be read in Å).

## General settings

The general simulation settings dialog can be found under the <code>Simulation &rarr; General</code> menu. There are only a few options here, but they can impact the simulations significantly. Firstly, the option to choose the `Potentials` used in the simulation is placed here (**Not yet supported**), the default being Kirkland's potentials from his [book](https://www.springer.com/gb/book/9781441965325).

Next is the `Full 3D integrals` setting. This is only used when the simulation is using the Full 3D method and it dictates how many sub-slices are used to approximate the 3D nature of a single slice. Higher values give a better approximation, but also take longer to simulate.

<div class="image-figure">
    <img style="width:500px;" src="{{'guide/assets/images/multistem-demo.png' | relative_url}}" alt="General dialog">
  <p>
      <span class="figure-title">Figure</span> Example of a STEM image being formed from individual simulations of parallel pixels that are summed to form a complete image.
  </p>
  </div> 

Finally is an option to set the STEM `Parallel pixels`. This option enables you to speed up STEM simulations by propagating multiple beam positions through the sample at once. This can greatly speed up simulations due to the need to calculate the potentials only once, but also has several caveats.

 1. TDS simulations will be less accurate. Due to the fact that multiple beam positions are propagated at once, the thermal vibrations approximation will not be as valid. If the parallel pixels are a significant fraction of the total STEM pixels, the TDS simulation will not be valid. The parallel pixels themselves are chosen at random (i.e. not all next to each other) to try and reduce this effect.
 2. Increasing this value can crash the program or even your computer if it is set too high. Increasing this value consumes more memory on the selected device and is possible to increase it too much. You will need to find out what value is acceptable on your hardware

<div class="image-figure">
    <img style="width:300px;" src="{{'guide/assets/images/general-dialog.png' | relative_url}}" alt="General dialog">
  <p>
      <span class="figure-title">Figure</span> General settings dialog.
  </p>
  </div> 

## Simulation Area

Opening the area dialog through the <code>Simulation &rarr; Area</code> menu or the `Set area` buttons allows you to set the area to be simulated. The areas behave differently for each mode. At the bottom of the dialog, all the relevant scales and areas are shown.

### TEM
<div class="image-figure">
    <img style="width:300px;" src="{{'guide/assets/images/area-ctem.png' | relative_url}}" alt="TEM area">
<p>
    <span class="figure-title">Figure</span> The area options for the conventional TEM simulations. Note that the same range is applied to both x and y directions. 
</p>
</div> 

In TEM, the area is set very simply, defined by a start and finish position, but with a few caveats. Because the simulation relies on square FFTs, the simulation width and height must be the same. Also, when padding is added to the image, the amount of padding is adjusted to be an integer number of pixels, which will affect the pixel scale.

### CBED
<div class="image-figure">
    <img style="width:300px;" src="{{'guide/assets/images/area-cbed.png' | relative_url}}" alt="CBED area">
<p>
    <span class="figure-title">Figure</span> The area options for the CBED simulations, note the extra padding option.
</p>
</div> 

CBED simulation are only defined by the position of the probe. The padding is automatically added, though an extra padding option is available to adjust the scale in reciprocal space. For example, a CBED disc may only have a radius of 10 mrad, but the simulation scale may only achieve 5 mrad per pixel. This will create a lot of error in simulation the disc (i.e. try drawing a disc in a 4×4 pixel image). In this case, the padding can be increased so that the pixel resolution is smaller and a more accurate disc can be formed.


### STEM
<div class="image-figure">
    <img style="width:300px;" src="{{'guide/assets/images/area-stem.png' | relative_url}}" alt="STEM area">
<p>
    <span class="figure-title">Figure</span> The area options for the STEM simulations, the extra padding option behaves the same as in CBED.
</p>
</div> 

In STEM mode, the scan parameters are defined very simply as a start/finish position as well as the number of pixels. This image is not directly affected by the simulation resolution (as opposed to the scan resolution), so can be any size you wish. The simulation resolution does affect the reciprocal resolution achieved, for example, for annular dark field simulations, a higher resolution may be needed to achieve the large collection angles. Equally, the simulation resolution is affected by the range of the scan. A padding option is provided, for the same reason as CBED.

## Default settings

clTEM supports user defined default configurations. In the install directory there should be a `microscopes` folder (create it if it doesn't exist). Inside you can save a `default.json` that will be loaded by default. This is useful for defining default microscope parameters, such as STEM detectors, so you don't have to define them every time.
