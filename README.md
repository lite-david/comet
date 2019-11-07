# Comet
RISC-V ISA based 32-bit processor written in C++ for High Level Synthesis (HLS).

## Pre-requisites
1. [RISC-V toolchain](https://github.com/freechipsproject/rocket-chip)
2. [Vivado HLS](https://www.xilinx.com/products/design-tools/vivado/integration/esl-design.html)
3. [Catapult HLS](https://www.mentor.com/hls-lp/catapult-high-level-synthesis/)

## Components of the project
The `synthesizable` directory contains C++ files which can be compiled to run as an emulator, synthesized for FPGA or ASIC. Vivado HLS is used for FPGA IP synthesis and Catapult HLS is used for ASIC synthesis.

* Run `make` in the synthesizable folder to generate the emulator `catapult.sim`.
* To build it as an FPGA IP, run `script.tcl` in Vivado HLS.
* To synthesize it to rtl for ASIC, run `directives.tcl` in Catapult HLS.

The `cycle_accurate_emulator` directory, simulates a Direct Mapped Cache and DRAM keeping the same core architecture. It can be used to run larger benchmarks whose data / instructions do not fit in 32KB. To build the emulator:

```
$ cd cycle_accurate_emulator
$ bash build.sh
```

This compiles the project and creates a folder `testdir`. To run the benchmarks:

```
$ cd testdir
$ bash testscript.sh
```
The `cache_synthesis_attempt` directory contains an attempt to synthesize the caching mechanism along with the pipelined core. It's currently under progress. 

## Note

This project is no longer maintained. For latest version, head over here - [Comet](https://gitlab.inria.fr/srokicki/Comet)
