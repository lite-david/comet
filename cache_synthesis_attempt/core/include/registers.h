#ifndef REGISTERS_H_
#define REGISTERS_H_

#include "portability.h"

struct FtoDC{
	CORE_UINT(32) pc;
	CORE_UINT(32) instruction; //Instruction to execute
};
	
struct DCtoEx{
	CORE_UINT(32) pc;
	CORE_INT(32) dataa; //First data from register file
	CORE_INT(32) datab; //Second data, from register file or immediate value
	CORE_INT(32) datac; 
	CORE_INT(32) datad; //Third data used only for store instruction and corresponding to rb
	CORE_INT(32) datae; 
	CORE_UINT(5) dest; //Register to be written
	CORE_UINT(7) opCode;//OpCode of the instruction
	CORE_INT(32) memValue; //Second data, from register file or immediate value	
	CORE_UINT(7) funct3 ;
    CORE_UINT(7) funct7;
    CORE_UINT(7) funct7_smaller ;
    CORE_UINT(6) shamt;
    CORE_UINT(5) rs1;
    CORE_UINT(5) rs2;        
};
	
struct ExtoMem{
	CORE_UINT(32) pc;
	CORE_INT(32) result; //Result of the EX stage
	CORE_INT(32) datad;
	CORE_INT(32) datac; //Data to be stored in memory (if needed)
	CORE_UINT(5) dest; //Register to be written at WB stage
	CORE_UINT(1) WBena; //Is a WB is needed ?
	CORE_UINT(7) opCode; //OpCode of the operation
	CORE_INT(32) memValue; //Second data, from register file or immediate value
	CORE_UINT(5) rs2;
	CORE_UINT(7) funct3;
	CORE_UINT(2) sys_status;
};

struct MemtoWB{
	CORE_INT(32) result; //Result to be written back
	CORE_UINT(5) dest; //Register to be written at WB stage
	CORE_UINT(1) WBena; //Is a WB is needed ?
    CORE_UINT(7) opCode; 
    CORE_UINT(2) sys_status;
};

//CORE_INT(32) ins_memory[8192]; //Instruction Memory(byte addressable), so it is divided into 4 memory blocks to address 1 instruction
//CORE_INT(32) data_memory[8192];  // Data Memory, also divided into 4 memory blocks
#endif
