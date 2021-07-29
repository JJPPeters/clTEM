
<p align="center"> 
<img src="https://jjppeters.github.io/clTEM/assets/logo.svg" alt="Logo" width="256px"/>
</p>

# clTEM  
## About  
clTEM is an OpenCL accelerated multislice program for simulating images from a transmission electron microscope, originally written by [Dr Adam Dyson](https://github.com/ADyson) as part of his [PhD](http://wrap.warwick.ac.uk/72953/).  

## Citing

Please cite clTEM using the [Zenodo repository](https://doi.org/10.5281/zenodo.5116155), or the following paper:

> [Jonathan J. P. Peters, _A Fast Frozen Phonon Algorithm Using Mixed Static Potentials_, Ultramicroscopy (2021)](https://doi.org/10.1016/j.ultramic.2021.113364)

## Features
- OpenCL acceleration with multi-device support
- Open any structure defined by simple .xyz or .cif files
- View structure through a OpenGL accelerated model viewer (showing slice positions and simulation area)
- Use classic slice model with projected potentials or a full 3D approximation
- Use potential parameterisation by [Kirkland](https://www.springer.com/gp/book/9781441965325), [Peng et al.](dx.doi.org/10.1107/S0108767395014371) or [Lobato and Van Dyck](dx.doi.org/10.1107/S205327331401643X)
- Simulate inelastic scattering using the frozen phonons and plasmons
- Simulate CTEM exit wave, diffraction pattern and image
- Incorporate dose into CTEM simulation with detective quantum efficiency (DQE) and noise transfer function (NTF)
- Use as a command line interface or with a GUI
- Compatible with Linux and Windows

## Usage
Some documentation has been written on the [github pages](https://jjppeters.github.io/clTEM/guide) site to introduce the basic concepts.

## Installation
Currently clTEM has only been released in alpha. It is fully functional, though not thoroughly tested and bugs can be expected.

## Dependencies  
  
 - [OpenCL](https://www.khronos.org/opencl/) - parallel programming language  
 - [clFFT](https://github.com/clMathLibraries/clFFT) - OpenCL accelerated FFTs  
 - [Qt](http://www.qt.io/) - GUI  
 - [QCustomPlot](http://qcustomplot.com/) - plotting 
 - [LibTIFF](http://simplesystems.org/libtiff/) - TIFF input/output  
 - [JSON for Modern C++](https://github.com/nlohmann/json) - JSON file creation/parsing
 - [Easylogging++](https://github.com/zuhd-org/easyloggingpp) - Error and debug Logging
 - [Eigen](http://eigen.tuxfamily.org/) - Linear algebra library

