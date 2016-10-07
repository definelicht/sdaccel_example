Example SDAccel CMake Project
=============================

This project is intended as an example of how to configure, build and run projects using SDAccel.

The included kernel is a simple, statically scheduled floating point operation pipeline, adding and multiplying input numbers with constants that return them to their original value before outputting them.

Vivado HLS and SDAccel are required to compile the project.

To configure in a separate folder called "build":
* `mkdir build`
* `cd build`
* `cmake ../`

Available build targets are:
* `make csim` - run the program in software using provided testbench to verify semantics.
* `make cosim` - run hardware simulation using the provided testbench.
* `make synthesis` - run high level synthesis to obtain HDL files.
* `make hw` - build hardware kernel using the `xocc` compiler from SDAccel.
* `make run_hw` - convenience command to run the built hardware kernel using the provided program `RunHardware` (build using just `make` or `make RunHardware`).

When running the kernel, the `XILINX_SDACCEL` or `XILINX_OPENCL` environment variable must point to the SDAccel install directory.

The project uses some convenience headers found in the `include` directory.
