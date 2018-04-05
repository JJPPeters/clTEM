# clTEM
## About
clTEM is an OpenCL accelerated multislice program for simulating images from a transmission electron microscope, originally written by [Dr Adam Dyson](https://github.com/ADyson) as part of his [PhD](http://wrap.warwick.ac.uk/72953/).

## Dependencies

 - [OpenCL](https://www.khronos.org/opencl/) - parallel programming language
 - [clFFT](https://github.com/clMathLibraries/clFFT) - OpenCL accelerated FFTs
 - [Qt](http://www.qt.io/) - GUI
 - [QCustomPlot](http://qcustomplot.com/) - plotting
 - [Boost](https://www.boost.org/) - additional C++ libraries (used for backwards compatibility of the OpenCL wrapper)
 - [LibTIFF](http://simplesystems.org/libtiff/) - TIFF input/output
 - [JSON for Modern C++](https://github.com/nlohmann/json) - JSON file creation/parsing