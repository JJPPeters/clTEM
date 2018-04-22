
<p align="center"> 
<img src="https://jjppeters.github.io/clTEM/images/logo.svg" alt="Logo" style="width: 300px"/>
</p>

# clTEM  
## About  
clTEM is an OpenCL accelerated multislice program for simulating images from a transmission electron microscope, originally written by [Dr Adam Dyson](https://github.com/ADyson) as part of his [PhD](http://wrap.warwick.ac.uk/72953/).  

## Features
- OpenCL acceleration with multi-device support
- Open any structure defined by a simple .xyz file  
- Use classic slice model for potentials or full 3D approximation
- Simulate STEM/CBED using the frozen phonon model
- Simulate CTEM diffraction/exit wave/image
- Incorporate detective quantum efficiency (DQE) and noise transfer function (NTF) into CTEM simulation
- Use the command line or with the GUI
- Compatible with Linux and Windows

## Usage
### Structure files
Structure is provided in the form of .xyz files containing the atom positions in Ångströms with some custom modifications.
The basic format follows the rules:

- First line contains the number of atoms in the structure
- Second line contains modifiers for the structure
--  `occ` introduces an extra column defining the site occupancy. Atoms on the same site must be next to each other in the .xyz file
--`nm` reads the coordinates of the atoms in nm
- The rest of the file contains lines in for format `Element-symbol x y z occupancy`
-- for example `Pb 0.0 0.4 0.4 1.0` if the `occ` tag has been set

## Installation
Currently clTEM has not been released, it is however mostly functional if you wish to compile it yourself.

## Dependencies  
  
 - [OpenCL](https://www.khronos.org/opencl/) - parallel programming language  
 - [clFFT](https://github.com/clMathLibraries/clFFT) - OpenCL accelerated FFTs  
 - [Qt](http://www.qt.io/) - GUI  
 - [QCustomPlot](http://qcustomplot.com/) - plotting  
 - [Boost](https://www.boost.org/) - additional C++ libraries (used for backwards compatibility of the OpenCL wrapper)  
 - [LibTIFF](http://simplesystems.org/libtiff/) - TIFF input/output  
 - [JSON for Modern C++](https://github.com/nlohmann/json) - JSON file creation/parsing

