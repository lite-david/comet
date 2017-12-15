/* vim: set ts=4 nu ai: */
#include "riscvISA.h"
#include "registers.h"
#include "core.h"
#if defined(__SIMULATOR__) || defined(__DEBUG__)
	#include "debug.h"
	#include "syscall.h"
	#define print_simulator_output(...) PrintDebugStatements(__VA_ARGS__)
	#define EX_SYS_CALL() case RISCV_SYSTEM: \
				extoMem->result = solveSysCall(dctoEx.dataa, dctoEx.datab, dctoEx.datac, dctoEx.datad, dctoEx.datae, &extoMem->sys_status); \
				break;
	#define DC_SYS_CALL() case RISCV_SYSTEM: \
			dctoEx->dest = 10; \
			rs1 = 17; \
			rs2 = 10; \
			dctoEx->datac = (extoMem.dest == 11 && mem_lock < 2) ? extoMem.result : ((memtoWB.dest == 11 && mem_lock == 0) ? memtoWB.result : REG[11]);\
			dctoEx->datad = (extoMem.dest == 12 && mem_lock < 2) ? extoMem.result : ((memtoWB.dest == 12 && mem_lock == 0) ? memtoWB.result : REG[12]);\
			dctoEx->datae = (extoMem.dest == 13 && mem_lock < 2) ? extoMem.result : ((memtoWB.dest == 13 && mem_lock == 0) ? memtoWB.result : REG[13]);\
			break;
	#define WB_SYS_CALL() if(memtoWB->sys_status == 1){\
			print_simulator_output("Exit system call received, Exiting... ");\
			*early_exit = 1;}\
			else if(memtoWB->sys_status == 2){\
			print_simulator_output("Unknown system call received, Exiting... ");\
			*early_exit = 1;}
#else
	#define print_simulator_output(...)
    #define EX_SYS_CALL()
	#define DC_SYS_CALL()
	#define WB_SYS_CALL()
#endif

#ifdef __DEBUG__
	#define print_debug(...) PrintDebugStatements(__VA_ARGS__)
	#define nl() PrintNewLine()
#else
	#define print_debug(...)
	#define nl()
#endif

#ifdef __VIVADO__
	#include "DataMemory.h"
	#define DO_MEM_PARAMETER DataMemory* data_memory
	#define MEM_SET(memory,address,value,op) memory->set(address,value,op)
	#define MEM_GET(memory,address,op,sign) memory->get(address,op,sign)
#endif

#ifdef __CATAPULT__
	#define DO_MEM_PARAMETER CORE_INT(32) data_memory[8192]
	#define MEM_SET(memory,address,value,op) memorySet(memory,address,value,op)
	#define MEM_GET(memory,address,op,sign) memoryGet(memory,address,op,sign)
#endif

CORE_INT(32) REG[32]; // Register file
CORE_UINT(2) sys_status;

void memorySet(CORE_INT(32) memory[8192], CORE_UINT(32) address, CORE_INT(32) value, CORE_UINT(2) op){
	CORE_UINT(13) wrapped_address = address % 8192;
	CORE_INT(32) byte0 = value.SLC(8,0);
	CORE_INT(32) byte1 = value.SLC(8,8);
	CORE_INT(32) byte2 = value.SLC(8,16);
	CORE_INT(32) byte3 = value.SLC(8,24);
	CORE_UINT(2) offset = wrapped_address & 0x3;
	CORE_UINT(13) location = wrapped_address >> 2;
	/*this wont synthesize in catapult */
	#ifdef __SIMULATOR__
	//first read existing value
	CORE_INT(32) memory_val = memory[location];
	CORE_INT(32) shifted_byte;
	CORE_INT(32) mask[4];
	mask[0] = 0xFFFFFF00;
	mask[1] = 0xFFFF00FF;
	mask[2] = 0xFF00FFFF;
	mask[3] = 0x00FFFFFF;
	CORE_INT(32) val_to_be_written;
	val_to_be_written = (memory_val & mask[offset]) | (byte0 << (offset*8));
	if(op & 1){
		val_to_be_written = (val_to_be_written & mask[offset+1]) | (byte1 << ((offset+1)*8));
	}
	if(op & 2){
		val_to_be_written = value;
	}
	memory[location] = val_to_be_written;
	#endif
	/*this will synthesize in catapult */
	#ifdef __CATAPULT__
	memory[location] = value;
	#endif
}

CORE_INT(32) memoryGet(CORE_INT(32) memory[8192], CORE_UINT(32) address, CORE_UINT(2) op, CORE_UINT(1) sign){
	CORE_UINT(13) wrapped_address = address % 8192;
	CORE_UINT(2) offset = wrapped_address & 0x3;
	CORE_UINT(13) location = wrapped_address >> 2;
	CORE_INT(32) result;
	result = sign ? -1 : 0;
	CORE_INT(32) mem_read = memory[location];
	CORE_INT(8) byte0 = mem_read.SLC(8,0);
	CORE_INT(8) byte1 = mem_read.SLC(8,8);
	CORE_INT(8) byte2 = mem_read.SLC(8,16);
	CORE_INT(8) byte3 = mem_read.SLC(8,24);

	switch(offset){
		case 0:
			break;
		case 1:
			byte0 = byte1;
			break;
		case 2:
			byte0 = byte1;
			byte1 = byte3;
			break;
		case 3:
			byte0 = byte3;
			break;
	}
	result.SET_SLC(0,byte0);
	if(op & 1){
		result.SET_SLC(8,byte1);
	}
	if(op & 2){
		result.SET_SLC(16,byte2);
		result.SET_SLC(24,byte3);
	}
	return result;
}

CORE_INT(32) reg_controller(CORE_UINT(32) address, CORE_UINT(1) op, CORE_INT(32) val){
	CORE_INT(32) return_val = 0;
	switch(op){
		case 0:
			REG[address % 32] = val;
			break;
		case 1:
			return_val = REG[address % 32];
			break;
	}
	return return_val;
}

void Ft(CORE_UINT(32) *pc, CORE_UINT(1) freeze_fetch, struct ExtoMem extoMem, CORE_INT(32) ins_memory[8192], struct FtoDC *ftoDC, CORE_UINT(3) mem_lock){

	CORE_UINT(32) next_pc;
	if(freeze_fetch){
		next_pc =  *pc;
	}
	else{
		next_pc = *pc + 4;
	}
	CORE_UINT(32) jump_pc;
	if(mem_lock > 1){
		jump_pc = next_pc;
	}
	else{
		jump_pc = extoMem.memValue;
	}
	CORE_UINT(1) control = 0;
	switch(extoMem.opCode){
		case RISCV_BR:
			control = extoMem.result > 0 ? 1 : 0;
			break;
		case RISCV_JAL:
			control = 1;
			break;
		case RISCV_JALR:
			control = 1;
			break;
		default:
			control = 0;
			break;
	}
	if(!freeze_fetch){
		(ftoDC->instruction).SET_SLC(0, ins_memory[(*pc & 0x0FFFF)/4]);
		ftoDC->pc=*pc;
		//debugger->set(debugger_index,ftoDC->pc,15);
	}
	*pc = control ? jump_pc : next_pc;
}


void DC(struct FtoDC ftoDC, struct ExtoMem extoMem, struct MemtoWB memtoWB, struct DCtoEx *dctoEx, CORE_UINT(7) *prev_opCode,
 CORE_UINT(32) *prev_pc, CORE_UINT(3) mem_lock, CORE_UINT(1) *freeze_fetch, CORE_UINT(1) *ex_bubble){

	CORE_UINT(5) rs1 = ftoDC.instruction.SLC(5,15);       // Decoding the instruction, in the DC stage
	CORE_UINT(5) rs2 = ftoDC.instruction.SLC(5,20);
	CORE_UINT(5) rd = ftoDC.instruction.SLC(5,7);
	CORE_UINT(7) opcode = ftoDC.instruction.SLC(7,0);
	CORE_UINT(7) funct7 = ftoDC.instruction.SLC(7,25);
	CORE_UINT(7) funct3 = ftoDC.instruction.SLC(3,12);
	CORE_UINT(7) funct7_smaller = 0;
	funct7_smaller.SET_SLC(1, ftoDC.instruction.SLC(6,26));
	CORE_UINT(6) shamt = ftoDC.instruction.SLC(6,20);
	CORE_UINT(13) imm13 = 0;
	imm13[12] = ftoDC.instruction[31];
	imm13.SET_SLC(5, ftoDC.instruction.SLC(6,25));
	imm13.SET_SLC(1, ftoDC.instruction.SLC(4,8));
	imm13[11] = ftoDC.instruction[7];
	CORE_INT(13) imm13_signed = 0;
	imm13_signed.SET_SLC(0, imm13);
	CORE_UINT(12) imm12_I = ftoDC.instruction.SLC(12,20);
	CORE_INT(12) imm12_I_signed = ftoDC.instruction.SLC(12,20);
	CORE_UINT(21) imm21_1 = 0;
	imm21_1.SET_SLC(12, ftoDC.instruction.SLC(8,12));
	imm21_1[11] = ftoDC.instruction[20];
	imm21_1.SET_SLC(1, ftoDC.instruction.SLC(10,21));
	imm21_1[20] = ftoDC.instruction[31];
	CORE_INT(21) imm21_1_signed = 0;
	imm21_1_signed.SET_SLC(0, imm21_1);
	CORE_INT(32) imm31_12 = 0;
	imm31_12.SET_SLC(12, ftoDC.instruction.SLC(20,12));
	CORE_UINT(1) forward_rs1;
	CORE_UINT(1) forward_rs2;
	CORE_UINT(1) forward_ex_or_mem_rs1;
	CORE_UINT(1) forward_ex_or_mem_rs2;
	CORE_UINT(1) datab_fwd = 0;
	CORE_INT(12) store_imm = 0;
	store_imm.SET_SLC(0,ftoDC.instruction.SLC(5,7));
	store_imm.SET_SLC(5,ftoDC.instruction.SLC(7,25));

	CORE_INT(32) reg_rs1 = reg_controller(rs1,1,0);
	CORE_INT(32) reg_rs2 = reg_controller(rs2,1,0);

	dctoEx->opCode=opcode;
	dctoEx->funct3=funct3;
	dctoEx->funct7_smaller=funct7_smaller;
	dctoEx->funct7=funct7;
	dctoEx->shamt=shamt;
	dctoEx->rs1=rs1;
	dctoEx->rs2=0;
	dctoEx->pc=ftoDC.pc;
	*freeze_fetch = 0;
	switch (opcode){
		case RISCV_LUI:
			dctoEx->dest = rd;
	        dctoEx->datab = imm31_12;
			break;
		case RISCV_AUIPC:
	        dctoEx->dest=rd;
	        dctoEx->datab = imm31_12;
			break;
		case RISCV_JAL:
	        dctoEx->dest=rd;
	        dctoEx->datab = imm21_1_signed;
			break;
		case RISCV_JALR:
			dctoEx->dest=rd;
	        dctoEx->datab = imm12_I_signed;
			break;
		case RISCV_BR:
			dctoEx->rs2=rs2;
	        datab_fwd = 1;
	        dctoEx->datac = imm13_signed;
	        dctoEx->dest = 0;
			break;
		case RISCV_LD:
    		dctoEx->dest=rd;
  			dctoEx->memValue = imm12_I_signed;
			break;
		case RISCV_ST:
			dctoEx->datad = rs2;
  			dctoEx->memValue = store_imm;
  			dctoEx->dest = 0;
			break;
		case RISCV_OPI:
        	dctoEx->dest = rd;
        	dctoEx->memValue = imm12_I_signed;
        	dctoEx->datab = imm12_I;
			break;
		case RISCV_OP:
			dctoEx->rs2=rs2;
			datab_fwd = 1;
        	dctoEx->dest=rd;
			break;
		DC_SYS_CALL()
	}

	forward_rs1 = ((extoMem.dest == rs1 && mem_lock < 2) || (memtoWB.dest == rs1 && mem_lock == 0)) ? 1 : 0;
	forward_rs2 = ((extoMem.dest == rs2 && mem_lock < 2) || (memtoWB.dest == rs2 && mem_lock == 0)) ? 1 : 0;
	forward_ex_or_mem_rs1 = (extoMem.dest == rs1) ? 1 : 0;
	forward_ex_or_mem_rs2 = (extoMem.dest == rs2) ? 1 : 0;

	dctoEx->dataa = (forward_rs1 && rs1 != 0) ? (forward_ex_or_mem_rs1 ? extoMem.result : memtoWB.result) : reg_rs1;
	if(opcode == RISCV_ST){
		dctoEx->datac = (forward_rs2 && rs2 != 0) ? (forward_ex_or_mem_rs2 ? extoMem.result : memtoWB.result) : reg_rs2;
	}
	if(datab_fwd){
		dctoEx->datab = (forward_rs2 && rs2 != 0) ? (forward_ex_or_mem_rs2 ? extoMem.result : memtoWB.result) : reg_rs2;
	}

	if(*prev_opCode == RISCV_LD && (extoMem.dest == rs1 || (opcode != RISCV_LD && extoMem.dest == rs2)) && mem_lock < 2 && *prev_pc != ftoDC.pc){
		*freeze_fetch = 1;
		*ex_bubble = 1;
	}
	*prev_opCode = opcode;
	*prev_pc = ftoDC.pc;
}



void Ex(struct DCtoEx dctoEx, struct ExtoMem *extoMem, CORE_UINT(1) *ex_bubble, CORE_UINT(1) *mem_bubble, CORE_UINT(2) *sys_status){

		CORE_UINT(32) unsignedReg1;
		unsignedReg1.SET_SLC(0,(dctoEx.dataa).SLC(32,0));
		CORE_UINT(32) unsignedReg2;
		unsignedReg2.SET_SLC(0,(dctoEx.datab).SLC(32,0));
		CORE_INT(33) mul_reg_a;
		CORE_INT(33) mul_reg_b;
		CORE_INT(66) longResult;
		CORE_INT(33) srli_reg;
		CORE_INT(33) srli_result;                   // Execution of the Instruction in EX stage
		extoMem->opCode= dctoEx.opCode;
		extoMem->dest=dctoEx.dest;
		extoMem->datac= dctoEx.datac;
		extoMem->funct3= dctoEx.funct3;
		extoMem->datad= dctoEx.datad;
		extoMem->sys_status = 0;
		if ((extoMem->opCode != RISCV_BR) && (extoMem->opCode != RISCV_ST)){
			extoMem->WBena = 1;
		}
		else{
			extoMem->WBena = 0;
		}

		switch (dctoEx.opCode){
			case RISCV_LUI:
		        extoMem->result = dctoEx.datab;
				break;
			case RISCV_AUIPC:
		       	extoMem->result = dctoEx.pc + dctoEx.datab;
				break;
			case RISCV_JAL:
		        extoMem->result = dctoEx.pc + 4;
				extoMem->memValue = dctoEx.pc + dctoEx.datab;
				break;
			case RISCV_JALR:
		        extoMem->result = dctoEx.pc + 4;
				extoMem->memValue = (dctoEx.dataa + dctoEx.datab) & 0xfffffffe;
				break;
			case RISCV_BR: // Switch case for branch instructions
				switch(dctoEx.funct3){
					case RISCV_BR_BEQ:
	        			extoMem->result = (dctoEx.dataa== dctoEx.datab);
						break;
					case RISCV_BR_BNE:
		        		extoMem->result = (dctoEx.dataa!= dctoEx.datab);
						break;
					case RISCV_BR_BLT:
						extoMem->result = (dctoEx.dataa< dctoEx.datab);
						break;
					case RISCV_BR_BGE:
	        			extoMem->result = (dctoEx.dataa>= dctoEx.datab);
						break;
					case RISCV_BR_BLTU:
					    extoMem->result = (unsignedReg1 < unsignedReg2);
						break;
					case RISCV_BR_BGEU:
					    extoMem->result = (unsignedReg1 >= unsignedReg2);
						break;
				}
				extoMem->memValue = dctoEx.pc + dctoEx.datac;
				break;
			case RISCV_LD:
				extoMem->result = (dctoEx.dataa + dctoEx.memValue);
				break;
			case RISCV_ST:
				extoMem->result = (dctoEx.dataa + dctoEx.memValue);
				extoMem->datac = dctoEx.datac;
				extoMem->rs2 = dctoEx.rs2;
				break;
			case RISCV_OPI:
				switch(dctoEx.funct3){
					case RISCV_OPI_ADDI:
						extoMem->result = dctoEx.dataa + dctoEx.memValue;
						break;
					case RISCV_OPI_SLTI:
						extoMem->result = (dctoEx.dataa < dctoEx.memValue) ? 1 : 0;
						break;
					case RISCV_OPI_SLTIU:
						extoMem->result = (unsignedReg1 < dctoEx.datab) ? 1 : 0;
						break;
					case RISCV_OPI_XORI:
						extoMem->result = dctoEx.dataa ^ dctoEx.memValue;
						break;
					case RISCV_OPI_ORI:
						extoMem->result =  dctoEx.dataa | dctoEx.memValue;
						break;
					case RISCV_OPI_ANDI:
						extoMem->result = dctoEx.dataa & dctoEx.memValue;
						break;
					case RISCV_OPI_SLLI:
						extoMem->result=  dctoEx.dataa << dctoEx.shamt;
						break;
					case RISCV_OPI_SRI:
						if (dctoEx.funct7_smaller == RISCV_OPI_SRI_SRLI){
							srli_reg.SET_SLC(0,dctoEx.dataa);
							srli_result = srli_reg >> dctoEx.shamt;
							extoMem->result = srli_result.SLC(32,0);
						}
						else //SRAI
							extoMem->result = dctoEx.dataa >> dctoEx.shamt;
						break;
				}
				break;
			case RISCV_OP:
				if (dctoEx.funct7 == 1){
					mul_reg_a = dctoEx.dataa;
					mul_reg_b = dctoEx.datab;
					mul_reg_a[32] = dctoEx.dataa[31];
					mul_reg_b[32] = dctoEx.datab[31];
					switch (dctoEx.funct3){ //Switch case for multiplication operations (RV32M)
						case RISCV_OP_M_MULHSU:
							mul_reg_b[32] = 0;
							break;
						case RISCV_OP_M_MULHU:
							mul_reg_a[32] = 0;
							mul_reg_b[32] = 0;
							break;
					}
					longResult = mul_reg_a * mul_reg_b;
					if(dctoEx.funct3 == RISCV_OP_M_MUL){
						extoMem->result = longResult.SLC(32,0);
					}
					else{
						extoMem->result = longResult.SLC(32,32);
					}
				}
				else{
					switch(dctoEx.funct3){
						case RISCV_OP_ADD:
							if (dctoEx.funct7 == RISCV_OP_ADD_ADD)
								extoMem->result = dctoEx.dataa + dctoEx.datab;
							else
				        		extoMem->result = dctoEx.dataa - dctoEx.datab;
							break;
						case RISCV_OP_SLL:
							extoMem->result = dctoEx.dataa << (dctoEx.datab & 0x3f);
							break;
						case RISCV_OP_SLT:
							extoMem->result = (dctoEx.dataa < dctoEx.datab) ? 1 : 0;
							break;
						case RISCV_OP_SLTU:
							extoMem->result = (unsignedReg1 < unsignedReg2) ? 1 : 0;
							break;
						case RISCV_OP_XOR:
							extoMem->result = dctoEx.dataa ^ dctoEx.datab;
							break;
						case RISCV_OP_OR:
							extoMem->result = dctoEx.dataa | dctoEx.datab;
							break;
						case RISCV_OP_AND:
							extoMem->result =  dctoEx.dataa & dctoEx.datab;
							break;
					}
				}
			break;
			EX_SYS_CALL()
		}
		
		if(*ex_bubble){
			*mem_bubble = 1;
			extoMem->pc = 0;
			extoMem->result = 0; //Result of the EX stage
			extoMem->datad = 0;
			extoMem->datac = 0;
			extoMem->dest = 0;
			extoMem->WBena = 0;
			extoMem->opCode = 0;
			extoMem->memValue = 0;
			extoMem->rs2 = 0;
			extoMem->funct3 = 0;
		}
		*ex_bubble = 0;
}

void do_Mem(DO_MEM_PARAMETER, struct ExtoMem extoMem,
 struct MemtoWB *memtoWB, CORE_UINT(3) *mem_lock, CORE_UINT(1) *mem_bubble, CORE_UINT(1) *wb_bubble){

	if(*mem_bubble){
		*mem_bubble = 0;
		//*wb_bubble = 1;
		memtoWB->result = 0; //Result to be written back
		memtoWB->dest = 0; //Register to be written at WB stage
		memtoWB->WBena = 0; //Is a WB is needed ?
    	memtoWB->opCode = 0;
    	memtoWB->sys_status = 0;
	}
	else{
		if(*mem_lock > 0){
			*mem_lock = *mem_lock - 1;
			memtoWB->WBena = 0;
		}

		memtoWB->sys_status = extoMem.sys_status;
		memtoWB->opCode=extoMem.opCode;
		memtoWB->result = extoMem.result;
	 	CORE_INT(32) result;
		CORE_UINT(2) st_op = 0;
		CORE_UINT(2) ld_op = 0;
		CORE_UINT(1) sign = 0;

		if(*mem_lock == 0){
			memtoWB->WBena = extoMem.WBena;
			memtoWB->dest = extoMem.dest; // Memory operaton in do_Mem stage
			switch(extoMem.opCode){
 				case RISCV_BR:
					if (extoMem.result){
						*mem_lock = 3;
					}
					memtoWB->WBena = 0;
					memtoWB->dest = 0;
					break;
				case RISCV_JAL:
					*mem_lock = 3;
					break;
				case RISCV_JALR:
					*mem_lock = 3;
					break;
				case RISCV_LD:
					switch(extoMem.funct3){
						case RISCV_LD_LW:
							ld_op = 3;
							sign = 1;
							break;
						case RISCV_LD_LH:
							ld_op = 1;
							sign = 1;
							break;
						case RISCV_LD_LHU:
							ld_op = 1;
							sign = 0;
							break;
						case RISCV_LD_LB:
							ld_op = 0;
							sign = 1;
							break;
						case RISCV_LD_LBU:
							ld_op = 1;
							sign = 0;
							break;
		           		 }
						memtoWB->result = MEM_GET(data_memory,memtoWB->result,ld_op,sign);
		           		break;
				case RISCV_ST:
			   		switch(extoMem.funct3){
						case RISCV_ST_STW:
							st_op = 3;
                            break;
                        case RISCV_ST_STH:
                        	st_op = 1;
                           	break;
                        case RISCV_ST_STB:
                        	st_op = 0;
                        	break;
                    }
					MEM_SET(data_memory,memtoWB->result,extoMem.datac,st_op);
					//data_memory[(memtoWB->result/4)%8192] = extoMem.datac;
			   		break;
			}
		}
	}
}


void doWB(struct MemtoWB *memtoWB, CORE_UINT(1) *wb_bubble, CORE_UINT(1) *early_exit){
		if (memtoWB->WBena == 1 && memtoWB->dest != 0){
			reg_controller(memtoWB->dest, 0, memtoWB->result);
		}
		WB_SYS_CALL()
}
#pragma SDS data zero_copy(dm[0:8192])
void doStep(CORE_UINT(32) pc, CORE_UINT(32) nbcycle, CORE_INT(32) ins_memory[8192],
	CORE_INT(32) dm[8192], CORE_INT(32) dm_out[8192]){//, CORE_INT(32) debug_arr[200]){

	int i;

	#ifdef __VIVADO__
	DataMemory data_memory;
    for(i = 0;i<8192;i++){
		#pragma HLS PIPELINE
    	data_memory.memory[i][0]=dm[i].SLC(8,0);
    	data_memory.memory[i][1]=dm[i].SLC(8,8);
    	data_memory.memory[i][2]=dm[i].SLC(8,16);
    	data_memory.memory[i][3]=dm[i].SLC(8,24);
    }
	#endif

	struct MemtoWB memtoWB;
	struct ExtoMem extoMem;
	struct DCtoEx dctoEx;
	struct FtoDC ftoDC;

	CORE_UINT(32) n_inst=0;
	CORE_UINT(7) prev_opCode=0;
	CORE_UINT(32) prev_pc = 0;

	#ifdef __DEBUG__
	CORE_UINT(32) debug_pc = 0;
	int reg_print_halt = 0;
	#endif

	CORE_UINT(32) ft_pc = 0;
	memtoWB.WBena=0;
	memtoWB.dest=0;
	memtoWB.opCode=0;
	extoMem.opCode=0;
	extoMem.sys_status = 0;
	CORE_UINT(3) mem_lock=0;
	dctoEx.opCode=0;
	CORE_UINT(1) early_exit = 0;

	dctoEx.dataa = 0; //First data from register file
	dctoEx.datab = 0; //Second data, from register file or immediate value
	dctoEx.datac = 0;
	dctoEx.datad = 0; //Third data used only for store instruction and corresponding to rb
	dctoEx.dest = 0; //Register to be written

	CORE_UINT(1) freeze_fetch = 0;
	CORE_UINT(1) ex_bubble = 0;
	CORE_UINT(1) mem_bubble = 0;
	CORE_UINT(1) wb_bubble = 0;

	for(i = 0;i<32;i++){
		#pragma HLS PIPELINE
		REG[i] = 0;
	}

	REG[2] = 0xf00000;
	sys_status = 0;

	doStep_label1:while(n_inst < nbcycle){
		#pragma HLS PIPELINE II=1
		#ifdef __DEBUG__
  			print_debug(n_inst, ";", std::hex, (int)pc, ";",	(int)ins_memory[(pc & 0x0FFFF)/4]," ");
		#endif
   	    doWB(&memtoWB, &wb_bubble, &early_exit);
		#ifdef __VIVADO__
			do_Mem(&data_memory, extoMem, &memtoWB, &mem_lock, &mem_bubble, &wb_bubble);
		#else
   			do_Mem(dm, extoMem, &memtoWB, &mem_lock, &mem_bubble, &wb_bubble);
		#endif
 		Ex(dctoEx, &extoMem, &ex_bubble, &mem_bubble, &sys_status);
		DC(ftoDC, extoMem, memtoWB, &dctoEx, &prev_opCode, &prev_pc, mem_lock, &freeze_fetch, &ex_bubble);
		Ft(&pc,freeze_fetch, extoMem, ins_memory, &ftoDC, mem_lock);
		n_inst++;
		#ifdef __DEBUG__
		for(i=0;i<32;i++){
			print_debug(";",std::hex,(int)REG[i]);
		}
		nl();
		#endif

		if(early_exit == 1)
			break;
	}
	
	#ifdef __VIVADO__
	for(i = 0;i<8192;i++){
		#pragma HLS PIPELINE
		dm_out[i].SET_SLC(0,data_memory.memory[i][0]);
		dm_out[i].SET_SLC(8,data_memory.memory[i][1]);
		dm_out[i].SET_SLC(16,data_memory.memory[i][2]);
		dm_out[i].SET_SLC(24,data_memory.memory[i][3]);
	}
	#endif
	/*for(i=0;i<32;i++){    
		#pragma HLS PIPELINE
		debug_arr[i].SET_SLC(0,REG[i]);
	}*/

	print_simulator_output("Successfully executed all instructions in ",n_inst," cycles");
}
