// vim: set ts=4 nu ai:
#include <dram.h>
#include <stdlib.h>
#include <iostream>

void Dram::setMemory(CORE_UINT(32) address, CORE_UINT(8) value){
	memory[address] = value;		
}

CORE_UINT(8) Dram::getMemory(CORE_UINT(32) address){
	return memory[address];
}

