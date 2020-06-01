---
title: Compiling
---

# {{page.title}}

clTEM has been compiled and tested on windows using g++ provided by [Mingw-w64](https://mingw-w64.org/doku.php) though [MSYS2](https://www.msys2.org/). [CMake](https://cmake.org/) is used to create the build files. The dependencies are listed below

 - [OpenCL](https://www.khronos.org/opencl/) - parallel programming language
 - [clFFT](https://github.com/clMathLibraries/clFFT) - OpenCL accelerated FFTs 
 - [Qt5](http://www.qt.io/) - GUI
 - [QCustomPlot](http://qcustomplot.com/) - plotting
 - [LibTIFF](http://simplesystems.org/libtiff/) - TIFF input/output
 - [JSON for Modern C++](https://github.com/nlohmann/json) - JSON file creation/parsing
 - [Easylogging++](https://github.com/amrayn/easyloggingpp) - Debug logging
 - [Eigen](http://eigen.tuxfamily.org/index.php?title=Main_Page) - Linear algebra