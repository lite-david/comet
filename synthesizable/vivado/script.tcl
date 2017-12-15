############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 2015 Xilinx Inc. All rights reserved.
############################################################
open_project doStep
set_top doStep
add_files ../src/core.cpp -cflags "-I../include -D__VIVADO__=1 -Wall -O0 -g -fmessage-length=0 -D__SDSCC__ -I/opt/Xilinx/SDSoC/2016.1/aarch32-linux/include -I/udd/emascare/Work/Code/Main/merge/sdsoc/riscv/src -D__SDSVHLS__ -w"
open_solution "solution"
set_part {xc7z010clg400-2}
create_clock -period 8.5 -name default
config_rtl -reset_level low
source "./directives.tcl"
#csim_design
csynth_design
#cosim_design
export_design -format ip_catalog

