############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 2015 Xilinx Inc. All rights reserved.
############################################################
set_directive_interface -mode bram "doStep" ins_memory
set_directive_resource -core RAM_1P "doStep" ins_memory
set_directive_interface -mode bram "doStep" dm
set_directive_resource -core RAM_1P "doStep" dm
set_directive_interface -mode bram "doStep" dm_out
set_directive_resource -core RAM_1P "doStep" dm_out
set_directive_latency -min 1 "doStep"

