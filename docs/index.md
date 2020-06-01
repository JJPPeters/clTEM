---
title: Home
---

# {{page.title}}

**clTEM is currently in development, many of the final features are present if not fully tested. Please report any bugs [here](https://github.com/JJPPeters/clTEM/issues).**

clTEM is an OpenCL accelerate implementation of the multislice algorithm for simulating transmission electron microscope images. The use of OpenCL means that the code will run on any hardware (AMD, Nvidia and Intel), particularly designed to exploit the power of GPUs. Some example images are shown below.

<div class="image-figure">
    <img style="width:300px;" src="{{'/assets/images/ctem.jpg' | relative_url}}" alt="TEM simulation">
    <img style="width:300px;" src="{{'/assets/images/cbed.jpg' | relative_url}}" alt="CBED simulation">
    <p>
        <span class="figure-title">Figure</span> Conventional TEM and CBED image from a simulated using an AMD Vega 56. The TEM image has simulated noise from a Gatan Orius camera and the CBED image has 10 TDS configurations.
    </p>
</div> 

## Features

The main features of clTEM are

  - Support for TEM, STEM and CBED simulations
  - OpenCL acceleration for quick simulations on various devices
  - Multi-device support - STEM and CBED simulation times can be drastically decreased
  - Thermal diffuse scattering simulation using the frozen phonon model
  - Plasmon simulations
  - Simple input files with full occupancy and thermal vibrations defined per atom. See [here]({{ site.baseurl }}/guide/input) for more
  - Option to use the classic projected potential model or a full 3D approximation
  - Incorporate detective quantum efficiency (DQE) and noise transfer function (NTF) into TEM simulations
  - Use via na intuitive GUI or via command line
  - Compatible with Linux and Windows

<div class="image-figure">
    <img style="width:300px;" src="{{'/assets/images/ui1.jpg' | relative_url}}" alt="clTEM interface theme">
    <img style="width:300px;" src="{{'/assets/images/ui2.jpg' | relative_url}}" alt="clTEM interface native">
    <p>
        <span class="figure-title">Figure</span> Example screenshots of the clTEM interface using the the custom and native themes. Both running on Windows 10.
    </p>
</div> 


### Background

The original code was written by Dr M. Adam Dyson and can be found [here](https://github.com/ADyson/clTEM). This code was partly written as part of his [PhD](http://wrap.warwick.ac.uk/72953/). Much of the code has been written with the help of Kirkland's excellent book:

> [Earl J. Kirkland, _Advanced Computing in Electron Microscopy_, 2nd edition (2010)](https://doi.org/10.1007/978-1-4419-6533-2)