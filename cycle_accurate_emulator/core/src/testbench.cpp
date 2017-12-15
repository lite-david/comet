// vim: set ts=4 nu ai:
#ifndef __SIMULATOR__
#include <lib/elfFile.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdio.h>
#include <map>
#include <iostream>
#include <string.h>
#include <core.h>
#include <portability.h>
#include "mc_scverify.h"

#ifdef __VIVADO__
	#include "DataMemory.h"
#else 
	#include "DataMemoryCatapult.h"
#endif

using namespace std;

class Simulator{

	private:
		//counters
		map<CORE_UINT(32), CORE_UINT(8) > ins_memorymap;
		map<CORE_UINT(32), CORE_UINT(8) > data_memorymap;
		CORE_UINT(32) nbcycle;
		CORE_UINT(32) ins_addr;

		CORE_UINT(32) pc;

	public:
		CORE_INT(32)* ins_memory;
		CORE_INT(32)* data_memory;

		Simulator(){
			ins_memory = (CORE_INT(32) *)malloc(8192 * sizeof(CORE_INT(32)));
			data_memory = (CORE_INT(32) *)malloc(8192 * sizeof(CORE_INT(32)));
			for(int i =0;i<8192;i++){
				ins_memory[i]=0;
				data_memory[i]=0;
			}
		}

		~Simulator(){
			free(ins_memory);
			free(data_memory);
		}

		void fillMemory(){
			//Check whether data memory and instruction memory from program will actually fit in processor.
			cout << ins_memorymap.size()<<endl;

			if(ins_memorymap.size() / 4 > 8192){
				printf("Error! Instruction memory size exceeded");
				exit(-1);
			}
			if(data_memorymap.size() / 4 > 8192){
				printf("Error! Data memory size exceeded");
				exit(-1);
			}

			//fill instruction memory
			for(map<CORE_UINT(32), CORE_UINT(8) >::iterator it = ins_memorymap.begin(); it!=ins_memorymap.end(); ++it){
				ins_memory[(it->first/4) % 8192].SET_SLC(((it->first)%4)*8,it->second);
			}

			//fill data memory
			for(map<CORE_UINT(32), CORE_UINT(8) >::iterator it = data_memorymap.begin(); it!=data_memorymap.end(); ++it){
				//data_memory.set_byte((it->first/4)%8192,it->second,it->first%4);
				data_memory[(it->first/4) % 8192].SET_SLC((it->first%4)*8,it->second);
			}
		}

		void setInstructionMemory(CORE_UINT(32) addr, CORE_INT(8) value){
			ins_memorymap[addr] = value;
		}

		void setDataMemory(CORE_UINT(32) addr, CORE_INT(8) value){
			data_memorymap[addr] = value;
		}

		CORE_INT(32)* getInstructionMemory(){

			return ins_memory;
		}


		CORE_INT(32)* getDataMemory(){
			return data_memory;
		}

		void setPC(CORE_UINT(32) pc){
			this->pc = pc;
		}

		CORE_UINT(32) getPC(){
			return pc;
		}

};


CCS_MAIN(int argv, char **argc){
	char* binaryFile = "/udd/emascare/Work/Code/Main/merge/benchmarks/matrixmultiply.out";
	ElfFile elfFile(binaryFile);
	Simulator sim;
	int counter = 0;
	for (unsigned int sectionCounter = 0;sectionCounter<elfFile.sectionTable->size(); sectionCounter++){
        ElfSection *oneSection = elfFile.sectionTable->at(sectionCounter);
        if(oneSection->address != 0 && oneSection->getName().compare(".text")){
            //If the address is not null we place its content into memory
            unsigned char* sectionContent = oneSection->getSectionCode();
            for (unsigned int byteNumber = 0;byteNumber<oneSection->size; byteNumber++){
            	counter++;
                sim.setDataMemory(oneSection->address + byteNumber, sectionContent[byteNumber]);
            }
        }

        if (!oneSection->getName().compare(".text")){
        	unsigned char* sectionContent = oneSection->getSectionCode();
            for (unsigned int byteNumber = 0;byteNumber<oneSection->size; byteNumber++){
                sim.setInstructionMemory((oneSection->address + byteNumber) & 0x0FFFF, sectionContent[byteNumber]);
            }
    	}
    }

    for (int oneSymbol = 0; oneSymbol < elfFile.symbols->size(); oneSymbol++){
		ElfSymbol *symbol = elfFile.symbols->at(oneSymbol);
		const char* name = (const char*) &(elfFile.sectionTable->at(elfFile.indexOfSymbolNameSection)->getSectionCode()[symbol->name]);
		if (strcmp(name, "_start") == 0){
			fprintf(stderr, "%s\n", name);
			sim.setPC(symbol->offset);
		}
	}


    sim.fillMemory();
//    CORE_INT(32)* dm_in;
//    dm_in = sim.getDataMemory();
    CORE_INT(32)* dm_out;
    CORE_INT(32)* debug_out;
    dm_out = (CORE_INT(32) *)malloc(8192 * sizeof(CORE_INT(32)));
    debug_out = (CORE_INT(32) *)malloc(200 * sizeof(CORE_INT(32)));
    int ins;
    //std::cin >> ins;
    CCS_DESIGN(doStep(sim.getPC(),16925,sim.getInstructionMemory(),sim.getDataMemory(),dm_out));//,debug_out));
    for(int i = 0;i<32;i++){
    	std::cout << std::dec << i << " : ";
    	std::cout << std::hex << debug_out[i] << std::endl;
    }
    std::cout << "dm" <<std::endl;
    /*for(int i = 0;i<8192;i++){
        	std::cout << std::dec << i << " : ";
        	std::cout << std::dec << dm_out[i] << std::endl;
 }*/
    free(dm_out);
    free(debug_out);
	CCS_RETURN(0);
}
#endif
