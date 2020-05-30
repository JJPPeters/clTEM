---
title: Compiling
---

# {{page.title}}

clTEM has been compiled and tested on windows using [Mingw-w64](https://mingw-w64.org/doku.php) and on linux using g++. [CMake](https://cmake.org/) is used to create the build files. The dependencies are listed below

 - [OpenCL](https://www.khronos.org/opencl/) - parallel programming language
 - [clFFT](https://github.com/clMathLibraries/clFFT) - OpenCL accelerated FFTs 
 - [Qt5](http://www.qt.io/) - GUI
 - [QCustomPlot](http://qcustomplot.com/) - plotting
 - [Boost](https://www.boost.org/) - additional C++ libraries [](used for backwards compatibility of the OpenCL wrapper)
 - [LibTIFF](http://simplesystems.org/libtiff/) - TIFF input/output
 - [JSON for Modern C++](https://github.com/nlohmann/json) - JSON file creation/parsing