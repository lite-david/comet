/* vim: set ts=4 nu ai: */
#include <isa/riscvISA.h>
#include <registers.h>
#include <core.h>
#include <cache.h>

#ifdef __SIMULATOR__
	CORE_UINT(32) counter_reg = 0;
	CORE_UINT(1) in_function_call = 0;
	CORE_UINT(32) branch_counter = 0;
	CORE_UINT(32) jump_counter = 0;
#endif

#ifdef __SIMULATOR__
	#include <debug.h>
	#include <syscall.h>
	#define print_simulator_output(...) PrintDebugStatements(__VA_ARGS__)
	#define print_debug(...) PrintDebugStatements(__VA_ARGS__)
	#define nl() PrintNewLine()
	#define EX_SYS_CALL()	case RISCV_SYSTEM: \
								extoMem->result = solveSysCall(dctoEx.dataa, \
								  dctoEx.datab, dctoEx.datac, dctoEx.datad, dctoEx.datae, \
								  &extoMem->sys_status); \
								break;

	#define DC_SYS_CALL() 	case RISCV_SYSTEM: \
								dctoEx->dest = 10; \
								rs1 = 17; \
								rs2 = 10; \
								dctoEx->datac = (extoMem.dest == 11 && mem_lock < 2) ? \
									extoMem.result : ((memtoWB.dest == 11 && mem_lock == 0) ? \
										memtoWB.result : REG[11]); \
								dctoEx->datad = (extoMem.dest == 12 && mem_lock < 2) ? \
									extoMem.result : ((memtoWB.dest == 12 && mem_lock == 0) ? \
										memtoWB.result : REG[12]);\
								dctoEx->datae = (extoMem.dest == 13 && mem_lock < 2) ? \
									extoMem.result : ((memtoWB.dest == 13 && mem_lock == 0) ? \
										memtoWB.result : REG[13]); \
								break;

	#define WB_SYS_CALL() 	if(memtoWB.sys_status == 1){\
								print_simulator_output("Exit system call received, Exiting... ");\
								*early_exit = 1;}\
							else if(memtoWB.sys_status == 2){\
								print_simulator_output("Unknown system call received, Exiting... ");\
								*early_exit = 1;}

	#define COUNT()	if(!dcache_miss && !icache_miss){ \
	  					switch(dctoEx.opCode){ \
	  						case RISCV_JAL: \
	  							if(in_function_call) \
									jump_counter = jump_counter + 1; \
	  							break; \
	  						case RISCV_JALR: \
	  							if(in_function_call) \
									jump_counter = jump_counter + 1; \
	  							break; \
	  						case RISCV_BR: \
	  							if(extoMem.result == 1 && in_function_call == 1) \
									branch_counter = branch_counter + 1; \
	  							break; \
	  					} \
  					} 

	#define DC_CUST0()	case RISCV_OP_CUST0: \
							counter_reg = n_inst - counter_reg; \
							in_function_call = 1-in_function_call; \
							break;
	#define EX_CUST0()  case RISCV_OP_CUST0: \
							break;
	#define MEM_CUST0()	case RISCV_OP_CUST0: \
							break;
#else
	#define print_simulator_output(...)
    #define EX_SYS_CALL()
	#define DC_SYS_CALL()
	#define WB_SYS_CALL()
	#define DC_CUST0()
	#define EX_CUST0()
	#define MEM_CUST0()
	#define COUNT()
	#define print_debug(...)
	#define nl()
#endif

CORE_INT(32) REG[32]; // Register file
CORE_UINT(2) sys_status;
CORE_UINT(32) n_inst=0;

CORE_UINT(TAGBITS) getTag(CORE_UINT(32) address){
	return address.SLC(TAGBITS,IDBITS+SETBITS);
}

CORE_UINT(SETBITS) getSet(CORE_UINT(32) address){
	return address.SLC(SETBITS,IDBITS);
}

CORE_UINT(IDBITS) getId(CORE_UINT(32) address){
	return address.SLC(IDBITS,0);
}


CORE_INT(32) memory_controller(struct Cache *cache, CORE_UINT(DRAM_WIDTH) draminput[DRAM_SIZE], CORE_UINT(32) address,
 CORE_UINT(32) value, CORE_UINT(1) load_or_store, CORE_UINT(2) width, CORE_UINT(1) sign, CORE_UINT(2)* cache_miss){

	CORE_UINT(TAGBITS) tag = getTag(address);
	CORE_UINT(SETBITS) set = getSet(address);
	CORE_UINT(IDBITS) id = getId(address);
	CORE_UINT(IDBITS) id_1 = getId(address)+1;
	CORE_UINT(IDBITS) id_2 = getId(address)+2;
	CORE_UINT(IDBITS) id_3 = getId(address)+3;
	CORE_UINT(1) tag_match = (tag == cache->index[set].tag) ? 1 : 0;
	CORE_UINT(1) dirty = cache->index[set].dirtybit;
	CORE_INT(32) result;
	result = sign ? -1 : 0;

	CORE_UINT(8) byte0 = 0 ;//value.SLC(8,0);
	CORE_UINT(8) byte1 = 0 ;//value.SLC(8,8);
	CORE_UINT(8) byte2 = 0 ;//value.SLC(8,16);
	CORE_UINT(8) byte3 = 0 ;//value.SLC(8,24);

	//CORE_UINT(IDBITS) zeroidbits = 0;
	//cache->dram_read_address.SET_SLC(0,zeroidbits);
	cache->dram_read_address = 0;
	cache->dram_read_address.SET_SLC(IDBITS,set);
	cache->dram_read_address.SET_SLC(IDBITS+SETBITS,tag);

	cache->dram_write_address = 0;
	//cache->dram_write_address.SET_SLC(0,zeroidbits);
	cache->dram_write_address.SET_SLC(IDBITS,set);
	cache->dram_write_address.SET_SLC(IDBITS+SETBITS,cache->index[set].tag);

	if(tag_match == 0 || cache->index[set].invalid==1){
		*cache_miss = 1;
		if(dirty){
			*cache_miss = 2;
		}
	}
	if(*cache_miss == 0){
		switch(load_or_store){
			case LOAD:
				byte0 = cache->data[set][id];
				byte1 = cache->data[set][id_1];
				byte2 = cache->data[set][id_2];
				byte3 = cache->data[set][id_3];
				break;
			case STORE:
				cache->index[set].dirtybit = 1;
				cache->data[set][id] = value.SLC(8,0);
				if(width & HALF_WORD_MASK){
					cache->data[set][id_1] = value.SLC(8,8);
				}
				if(width & WORD_MASK){
					cache->data[set][id_2] = value.SLC(8,16);
					cache->data[set][id_3] = value.SLC(8,24);
				}
				break;
		}
		result.SET_SLC(0,byte0);
		if(width & HALF_WORD_MASK){
			result.SET_SLC(8,byte1);
		}
		if(width & WORD_MASK){
			result.SET_SLC(16,byte2);
			result.SET_SLC(24,byte3);
		}
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

void Ft(CORE_UINT(32) *pc, CORE_UINT(1) freeze_fetch, struct ExtoMem extoMem,
	struct Cache* icache, struct FtoDC *ftoDC, CORE_UINT(3) mem_lock, CORE_UINT(2) dcache_miss,
	CORE_UINT(2) *icache_miss, CORE_UINT(DRAM_WIDTH) dram[DRAM_SIZE]){

	CORE_UINT(32) next_pc;
	CORE_UINT(32) ins;
	CORE_UINT(32) temp_pc;
	CORE_UINT(32) jump_pc;
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

	if(*icache_miss){
		print_debug("[ICache miss] ");
		control = 0;
	}

	if(freeze_fetch || dcache_miss || *icache_miss){
		next_pc =  *pc;
	}
	else{
		next_pc = *pc + 4;
	}

	if(mem_lock > 1){
		jump_pc = next_pc;
	}
	else{
		jump_pc = extoMem.memValue;
	}

	if(!freeze_fetch && !dcache_miss && !*icache_miss){
		ins = memory_controller(icache, dram,*pc,0,LOAD,WORD_WIDTH,0,icache_miss);
		if(!*icache_miss){
			(ftoDC->instruction).SET_SLC(0,ins);
			ftoDC->pc=*pc;
		}
		else{
			print_debug("[ICache miss] ");
		}
	}
	if(!*icache_miss)
		*pc = control ? jump_pc : next_pc;
	
}


void DC(struct FtoDC ftoDC, struct ExtoMem extoMem, struct MemtoWB memtoWB, struct DCtoEx *dctoEx,
	CORE_UINT(3) mem_lock, CORE_UINT(1) *freeze_fetch, CORE_UINT(1) *ex_bubble, CORE_UINT(2) dcache_miss,
	CORE_UINT(2) icache_miss){

	if(!dcache_miss && !icache_miss){
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

	CORE_UINT(7) prev_opCode = dctoEx->opCode;
	CORE_UINT(32) prev_pc = dctoEx->pc;
	dctoEx->opCode=opcode;
	dctoEx->pc=ftoDC.pc;

	CORE_INT(32) reg_rs1 = reg_controller(rs1,1,0);
	CORE_INT(32) reg_rs2 = reg_controller(rs2,1,0);

	dctoEx->funct3=funct3;
	dctoEx->funct7_smaller=funct7_smaller;
	dctoEx->funct7=funct7;
	dctoEx->shamt=shamt;
	dctoEx->rs1=rs1;
	dctoEx->rs2=0;
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
		DC_CUST0()
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

	if(prev_opCode == RISCV_LD && (extoMem.dest == rs1 || (opcode != RISCV_LD && extoMem.dest == rs2)) && mem_lock < 2 && prev_pc != ftoDC.pc){
		*freeze_fetch = 1;
		*ex_bubble = 1;
	}
	}
}



void Ex(struct DCtoEx dctoEx, struct ExtoMem *extoMem, CORE_UINT(1) *ex_bubble, CORE_UINT(1) *mem_bubble,
	CORE_UINT(2) *sys_status, CORE_UINT(2) dcache_miss, CORE_UINT(2) icache_miss){

		if(!dcache_miss && !icache_miss){
		CORE_UINT(32) unsignedReg1;
		unsignedReg1.SET_SLC(0,(dctoEx.dataa).SLC(32,0));
		CORE_UINT(32) unsignedReg2;
		unsignedReg2.SET_SLC(0,(dctoEx.datab).SLC(32,0));
		CORE_INT(33) mul_reg_a;
		CORE_INT(33) mul_reg_b;
		CORE_INT(66) longResult;
		CORE_INT(33) srli_reg = 0;
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
			EX_CUST0()
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
}

void do_Mem(struct Cache* dcache, struct ExtoMem extoMem,struct MemtoWB *memtoWB, CORE_UINT(3) *mem_lock,
	CORE_UINT(1) *mem_bubble, CORE_UINT(1) *wb_bubble, CORE_UINT(2)* dcache_miss, CORE_UINT(2) icache_miss,
	CORE_UINT(DRAM_WIDTH) dram[DRAM_SIZE]){

	if(!icache_miss){
		if(*dcache_miss == 0){
			if(*mem_bubble){
				*mem_bubble = 0;
				memtoWB->result = 0; 
				memtoWB->dest = 0; 
				memtoWB->WBena = 0; 
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
				CORE_UINT(2) width = 0;
				CORE_UINT(1) load_or_store = 0;
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
							load_or_store = LOAD;
							switch(extoMem.funct3){
								case RISCV_LD_LW:
									width = WORD_WIDTH;
									sign = 1;
									break;
								case RISCV_LD_LH:
									width = HALF_WORD_WIDTH;
									sign = 1;
									break;
								case RISCV_LD_LHU:
									width = HALF_WORD_WIDTH;
									sign = 0;
									break;
								case RISCV_LD_LB:
									width = BYTE_WIDTH;
									sign = 1;
									break;
								case RISCV_LD_LBU:
									width = BYTE_WIDTH;
									sign = 0;
									break;
				           		 }
				           		break;
						case RISCV_ST:
							load_or_store = STORE;
					   		switch(extoMem.funct3){
								case RISCV_ST_STW:
									width = WORD_WIDTH;
									sign = 0;
		                            break;
		                        case RISCV_ST_STH:
		                        	width = HALF_WORD_WIDTH;
		                        	sign = 0;
		                           	break;
		                        case RISCV_ST_STB:
		                        	width = BYTE_WIDTH;
		                        	sign = 0;
		                        	break;
		                    }
					   		break;
						MEM_CUST0()
					}
					if(extoMem.opCode == RISCV_LD || extoMem.opCode == RISCV_ST){
						memtoWB->result = memory_controller(dcache, dram, extoMem.result,
						 extoMem.datac, load_or_store, width, sign, dcache_miss);
						// if(*dcache_miss)
						// 	memtoWB->WBena = 0;
					}
				}
			}
		}
		else{
			// memtoWB->WBena = 0;
			print_debug("[DCache miss] ");
		}
	}
}

void doWB(struct MemtoWB memtoWB, CORE_UINT(1) *wb_bubble, CORE_UINT(1) *early_exit, CORE_UINT(2) icache_miss, CORE_UINT(2) dcache_miss){
	if (memtoWB.WBena == 1 && memtoWB.dest != 0 && icache_miss == 0 && dcache_miss == 0){
		reg_controller(memtoWB.dest, 0, memtoWB.result);
	}
	WB_SYS_CALL()
}

#pragma SDS data zero_copy(dram)
void doStep(CORE_UINT(32) pc, CORE_UINT(32) nbcycle, CORE_UINT(DRAM_WIDTH) dram[DRAM_SIZE]){

	int i;
	struct Cache icache,dcache;
	struct MemtoWB memtoWB;
	struct ExtoMem extoMem;
	struct DCtoEx dctoEx;
	struct FtoDC ftoDC;

	for(i = 0;i<SETS;i++){
		icache.index[i].tag = 0;
		icache.index[i].invalid = 1;
		icache.index[i].dirtybit = 0;
		dcache.index[i].tag = 0;
		dcache.index[i].invalid = 1;	
		dcache.index[i].dirtybit = 0;
	}
	icache.id_counter_1 = 0;
	dcache.id_counter_1 = 0;
	dcache.id_counter_2 = 0;

	CORE_UINT(32) ft_pc = 0;
	memtoWB.WBena=0;
	memtoWB.dest=0;
	memtoWB.opCode=0;
	extoMem.opCode=0;
	extoMem.sys_status = 0;
	CORE_UINT(3) mem_lock=0;
	dctoEx.opCode=0;
	dctoEx.pc=0;
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
	CORE_UINT(2) dcache_miss = 0;
	CORE_UINT(1) dummy_signal = 0;
	CORE_UINT(2) icache_miss = 0;

	for(i = 0;i<32;i++){
		REG[i] = 0;
	}

	REG[2] = 0xf00000;
	sys_status = 0;

	while(n_inst < nbcycle){
			
		#pragma HLS PIPELINE
  		print_debug(n_inst, ";");


   	    doWB(memtoWB, &wb_bubble, &early_exit,icache_miss,dcache_miss);
   	    if(dcache_miss > 0){
   	    	if(dcache_miss == 2)
   	    		dram[dcache.dram_write_address + dcache.id_counter_1] = dcache.data[getSet(dcache.dram_write_address)][dcache.id_counter_1];
   	    	dcache.data[getSet(dcache.dram_read_address)][dcache.id_counter_1] = dram[dcache.dram_read_address + dcache.id_counter_1];
   	    	dcache.id_counter_1++;
   	    	if(dcache.id_counter_1 >= CACHEBLOCKBYTES){
   	    		dcache_miss = 0;
   	    		dcache.id_counter_1 = 0;
   	    		dcache.index[getSet(dcache.dram_read_address)].tag = getTag(dcache.dram_read_address);
   	    		dcache.index[getSet(dcache.dram_read_address)].invalid = 0;
   	    		dcache.index[getSet(dcache.dram_read_address)].dirtybit = 0;
   	    	}
   	    }
   		do_Mem(&dcache, extoMem, &memtoWB, &mem_lock, &mem_bubble, &wb_bubble,&dcache_miss,icache_miss, dram);
 		Ex(dctoEx, &extoMem, &ex_bubble, &mem_bubble, &sys_status,dcache_miss,icache_miss);
		COUNT()
		DC(ftoDC, extoMem, memtoWB, &dctoEx, mem_lock, &freeze_fetch, &ex_bubble,dcache_miss,icache_miss);

		if(icache_miss == 1){
   	    	icache.data[getSet(icache.dram_read_address)][icache.id_counter_1] = dram[icache.dram_read_address + icache.id_counter_1];
   	    	icache.id_counter_1++;
   	    	if(icache.id_counter_1 >= CACHEBLOCKBYTES){
   	    		icache_miss = 0;
   	    		icache.id_counter_1 = 0;
   	    		icache.index[getSet(icache.dram_read_address)].tag = getTag(icache.dram_read_address);
   	    		icache.index[getSet(icache.dram_read_address)].invalid = 0;
   	    	}
   	    }
		Ft(&pc,freeze_fetch, extoMem, &icache, &ftoDC, mem_lock,dcache_miss, &icache_miss, dram);
  		print_debug(std::hex, (int)ftoDC.pc, ";",	(int)ftoDC.instruction," ");
		
		n_inst++;
		
		for(i=0;i<32;i++){
			print_debug(";",std::hex,(int)REG[i]);
		}
		nl();

		if(early_exit == 1)
			break;
	}

	print_simulator_output("cycle counter value: ",counter_reg);nl();
	print_simulator_output("number of branches taken: ",branch_counter);nl();
	print_simulator_output("number of jumps taken: ",jump_counter);
}
