/* vim: set ts=4 ai nu: */
#include <lib/elfFile.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <core.h>
#include <portability.h>
#include <vector>
#include <dram.h>
#include <cache.h>
#include <iomanip>
//#include "sds_lib.h"

#ifdef __VIVADO__
	#include "DataMemory.h"
#endif

using namespace std;
//cout << showbase << hex;
class Simulator{

	private:
		//counters
		Dram dram;
		Cache icache;
		Cache dcache;
		ElfFile elfFile;
		CORE_UINT(32) pc;

	public:

		Simulator(const char* binaryFile): elfFile(binaryFile), icache(&dram), dcache(&dram){

		}

		void loadElfIntoDram(){
			unsigned int sectionCounter;
			unsigned int sectionTableSize = elfFile.sectionTable->size();
			unsigned char* sectionContent;
			unsigned int byteNumber;

			for (sectionCounter = 0; sectionCounter < sectionTableSize;  sectionCounter++){
        		ElfSection *oneSection = elfFile.sectionTable->at(sectionCounter);
        		if(oneSection->address != 0 && oneSection->getName().compare(".text")){
            		//If the address is not null we place its content into memory
            		sectionContent = oneSection->getSectionCode();
            		for (byteNumber = 0; byteNumber<oneSection->size; byteNumber++){
						//cout << oneSection->address + byteNumber  << " : " << (CORE_UINT(8)) sectionContent[byteNumber] << endl;
           				dram.setMemory(oneSection->address + byteNumber, (CORE_UINT(8))sectionContent[byteNumber]); 
					}
       		 	}		

     		   	if(!oneSection->getName().compare(".text")){
        			sectionContent = oneSection->getSectionCode();
            		for (byteNumber = 0;byteNumber<oneSection->size; byteNumber++){
                		dram.setMemory(oneSection->address + byteNumber, sectionContent[byteNumber]);
            		}
    			}
    		}
		}

		Cache* getICache(){
			return &icache;
		}


		Cache* getDCache(){
			return &dcache;
		}

		void setPC(){
			int oneSymbol;
			const char* name;
   			for (oneSymbol = 0; oneSymbol < elfFile.symbols->size(); oneSymbol++){
				ElfSymbol *symbol = elfFile.symbols->at(oneSymbol);
				name = (const char*) &(elfFile.sectionTable->at(elfFile.indexOfSymbolNameSection)->getSectionCode()[symbol->name]);
				if (strcmp(name, "_start") == 0){
					pc = symbol->offset;
				}
			}
		}

		CORE_UINT(32) getPC(){
			return pc;
		}
};


int main(int argc, char** argv){
	const char* binaryFile;
	if(argc != 2)
		binaryFile = "benchmarks/build/median.out";
	else
		binaryFile = argv[1];

	cout  << hex;
	Simulator sim(binaryFile);
	sim.loadElfIntoDram();
	sim.setPC();
	
	//debugging 
	//sim.printMem();

    CORE_INT(32)* dm_out = (CORE_INT(32) *)malloc(8192 * sizeof(CORE_INT(32)));
    int ins = 1000000;
	//cout << "pc start is: " << (int)sim.getPC() << endl;
	
    doStep(sim.getPC(),ins,sim.getICache(),sim.getDCache(),dm_out);
    /*for(int i = 0;i<34;i++){ 
    	std::cout << std::dec << i << " : ";
    	std::cout << std::hex << debug_out[i] << std::endl;
    }*/
    //std::cout << "dm" <<std::endl;
    //for(int i = 0;i<8192;i++){
    //	   	std::cout << std::dec << i << " : ";
    //    	std::cout << std::dec << dm_out[i] << std::endl;
    // }
    free(dm_out);
	return 0;
}
