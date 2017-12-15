/* vim: set ts=4 ai nu: */
#include <lib/elfFile.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <core.h>
#include <portability.h>
//#include "sds_lib.h"

using namespace std;
//cout << showbase << hex;
class Simulator{

	private:
		//counters
		CORE_UINT(DRAM_WIDTH)* dram;
		ElfFile elfFile;
		CORE_UINT(32) pc;

	public:

		Simulator(const char* binaryFile): elfFile(binaryFile){
			dram = (CORE_UINT(DRAM_WIDTH)*)malloc(DRAM_SIZE* sizeof(CORE_UINT(DRAM_WIDTH)));
		}

		~Simulator(){
			free(dram);
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
           				dram[oneSection->address + byteNumber] = (CORE_UINT(8))sectionContent[byteNumber]; 

					}
       		 	}		

     		   	if(!oneSection->getName().compare(".text")){
        			sectionContent = oneSection->getSectionCode();
            		for (byteNumber = 0;byteNumber<oneSection->size; byteNumber++){
            			//cout << oneSection->address + byteNumber  << " : " << (CORE_UINT(8)) sectionContent[byteNumber] << endl;
                		dram[oneSection->address + byteNumber] = (CORE_UINT(8))sectionContent[byteNumber];
            		}
    			}
    		}
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

		CORE_UINT(DRAM_WIDTH)* getDram(){
			return dram;
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

    int ins = 1000000;
	// cout << "pc start is: " << (int)sim.getPC() << endl;
	
    doStep(sim.getPC(),ins,sim.getDram());
	return 0;
}
