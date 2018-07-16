
<p align="center"> 
<img src="https://jjppeters.github.io/clTEM/images/logo.svg" alt="Logo" width="300px"/>
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
Some documentation has been written on the [github pages](https://jjppeters.github.io/clTEM/guide) site to introduce the basic concepts.

## Installation
Currently clTEM has only been released in alpha. It is however mostly functional, though not entirely tested and bugs can be expected.

## Dependencies  
  
 - [OpenCL](https://www.khronos.org/opencl/) - parallel programming language  
 - [clFFT](https://github.com/clMathLibraries/clFFT) - OpenCL accelerated FFTs  
 - [Qt](http://www.qt.io/) - GUI  
 - [QCustomPlot](http://qcustomplot.com/) - plotting  
 - [Boost](https://www.boost.org/) - additional C++ libraries (used for backwards compatibility of the OpenCL wrapper)  
 - [LibTIFF](http://simplesystems.org/libtiff/) - TIFF input/output  
 - [JSON for Modern C++](https://github.com/nlohmann/json) - JSON file creation/parsing

