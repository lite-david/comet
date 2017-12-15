# comet
RISC-V ISA based 32-bit processor written in C++ for HLS

## Pre-requisites
RISC-V toolchain
Vivado HLS
Catapult HLS

## Components of the project

The synthesizable directory contains C++ files which can be compiled to run as an emulator, synthesized for FPGA or ASIC.

Run `make` in the synthesizable folder to generate the emulator `catapult.sim`.
To build it as an FPGA IP, run `script.tcl` in Vivado HLS.
To synthesize it to rtl for ASIC, run `directives.tcl` in Catapult HLS.

